#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <pthread.h>
#include <nlohmann/json.hpp>
#include <unistd.h> 
#include <csignal> 

using json = nlohmann::json;
using namespace std;

const string API_URL = "https://api.upstox.com/v2/market-quote/ltp?instrument_key=";
string AUTH_TOKEN = "Bearer ";
const int MAX_KEYS_PER_REQUEST = 500;

// Brokerage and charges constants
const double BROKERAGE = 0.0001; // Example value per transaction
const double STT = 0.001; // Securities Transaction Tax
const double EXCHANGE_FEES = 0.0000325; // Exchange transaction charge
const double GST = 0.18; // Goods and Services Tax
const double SEBI_FEES = 0.000001; // SEBI turnover fees


pthread_mutex_t print_mutex; // Mutec for printing on terminal
pthread_mutex_t mtx; // Mutex for synchronizing access to market_data
json market_data_nse, market_data_bse;


double MIN_PROFIT_PER_TRADE;
double CAPITAL;


string readAccessToken(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open " << filename << endl;
        return "";
    }

    string access_token;
    getline(file, access_token);
    file.close();

    if (access_token.empty()) {
        cerr << "Error: Access token file is empty!" << endl;
    }
    
    return access_token;
}


// Function to load instrument keys from CSV file
vector<pair<string, string>> loadInstrumentKeys(const string& filename) {
    vector<pair<string, string>> instrument_keys;
    ifstream file(filename);
    string line, name, nse_key, bse_key, tradingsymbol;
    getline(file, line); // Skip header
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, name, ',');
        getline(ss, nse_key, ',');
        getline(ss, bse_key, ',');
        getline(ss, tradingsymbol, ',');
        nse_key.erase(remove(nse_key.begin(), nse_key.end(), '"'), nse_key.end());
        bse_key.erase(remove(bse_key.begin(), bse_key.end(), '"'), bse_key.end());
        instrument_keys.push_back({nse_key, bse_key});
    }
    return instrument_keys;
}


// Function to write response data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch market data
string fetchMarketData(const string& instrument_keys) {
    CURL* curl = curl_easy_init();
    string readBuffer;
    if (curl) {
        string url = API_URL + instrument_keys;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: " + AUTH_TOKEN).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

struct ThreadArgs {
    vector<string> instrument_keys;
    bool is_nse;
};

// Thread function to fetch and store market data
void* fetchMarketDataThread(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    string keys = "";
    
    // Create comma-separated string of keys
    for (size_t i = 0; i < args->instrument_keys.size(); i++) {
        if (i > 0) keys += ",";
        keys += args->instrument_keys[i];
    }

    string response = fetchMarketData(keys);
    json parsed_data = json::parse(response, nullptr, false);
    
    parsed_data = parsed_data["data"];

    pthread_mutex_lock(&mtx); // Lock the mutex before modifying shared data
    if (args->is_nse) {
        market_data_nse.update(parsed_data);
    } else {
        market_data_bse.update(parsed_data);
    }
    pthread_mutex_unlock(&mtx); // Unlock the mutex

    delete args; // Free memory allocated for arguments
    return nullptr;
}

json mergeMarketData(const json& market_data_nse, const json& market_data_bse) {
    json merged_data;

    auto it_nse = market_data_nse.begin();
    auto it_bse = market_data_bse.begin();

    while (it_nse != market_data_nse.end() && it_bse != market_data_bse.end()) {
        // Extract symbol from NSE and BSE keys
        string key_nse = it_nse.key();
        string key_bse = it_bse.key();

        string symbol_nse = key_nse.substr(key_nse.find(":") + 1);
        string symbol_bse = key_bse.substr(key_bse.find(":") + 1);

        // Ensure both symbols are the same
        if (symbol_nse == symbol_bse) {
            merged_data[symbol_nse]["NSE"] = it_nse.value();
            merged_data[symbol_nse]["BSE"] = it_bse.value();
        } else {
            cout << "Mismatch found: " << symbol_nse << " vs " << symbol_bse << endl;
        }

        ++it_nse;
        ++it_bse;
    }
    return merged_data;
}


// Extract LTP
double extractLTP(const json& stock_data) {
    try {
        return stock_data["last_price"].get<double>();
    } catch (...) {
        cerr << "Error extracting LTP" << endl;
        return 0.0;
    }
}

// Calculate net profit after charges
double calculateNetProfit(double buy_price, double sell_price, int quantity) {
    double gross_profit = (sell_price - buy_price) * quantity;
    double turnover = (buy_price + sell_price) * quantity;
    double total_charges = (turnover * (BROKERAGE + STT + EXCHANGE_FEES + SEBI_FEES)) + (GST * BROKERAGE * turnover);
    return gross_profit - total_charges;
}

// Struct to pass data to threads
struct ThreadData {
    const json* market_data;
    int start, end;
};

// Simulate arbitrage trading
void* simulateArbitrage(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    auto& market_data = *(data->market_data);

    for (int i = data->start; i < data->end; ++i) {
        try {
            auto it = std::next(market_data.begin(), i);
            const std::string& symbol = it.key();
            const auto& stock = it.value();
            
            double ltp_nse = extractLTP(stock["NSE"]);
            double ltp_bse = extractLTP(stock["BSE"]);
            int quantity = 0;
            
            if (ltp_bse > ltp_nse && ltp_nse > 0) {
                quantity = CAPITAL / ltp_nse;
                double profit = calculateNetProfit(ltp_nse, ltp_bse, quantity);
                if (profit > MIN_PROFIT_PER_TRADE) {
                    pthread_mutex_lock(&print_mutex);
                    cout << "Arbitrage Opportunity for " << symbol << ": Buy on NSE at " << ltp_nse
                         << " and Sell on BSE at " << ltp_bse << " | Net Profit: " << profit << endl;
                    pthread_mutex_unlock(&print_mutex);
                }
            }
            else if (ltp_nse > ltp_bse && ltp_bse > 0) {
                quantity = CAPITAL / ltp_bse;
                double profit = calculateNetProfit(ltp_bse, ltp_nse, quantity);
                if (profit > MIN_PROFIT_PER_TRADE) {
                    pthread_mutex_lock(&print_mutex);
                    cout << "Arbitrage Opportunity for " << symbol << ": Buy on BSE at " << ltp_bse
                         << " and Sell on NSE at " << ltp_nse << " | Net Profit: " << profit << endl;
                    pthread_mutex_unlock(&print_mutex);
                }
            }
        } catch (...) {
            pthread_mutex_lock(&print_mutex);
            cerr << "Error processing stock data." << endl;
            pthread_mutex_unlock(&print_mutex);
        }
    }
    return nullptr;
}


bool keep_running = true; // Flag to control the loop

void signalHandler(int signum) {
    cout << "\nInterrupt signal (" << signum << ") received. Stopping...\n";
    keep_running = false;
}

int main() {
    string access_token = readAccessToken("access_token.txt");
    AUTH_TOKEN += access_token;

    cout << "Enter your capital: ";
    cin >> CAPITAL;
    cout << "Enter minimum profit per trade: ";
    cin >> MIN_PROFIT_PER_TRADE;

    vector<pair<string, string>> instrument_keys = loadInstrumentKeys("filtered_equity.csv");

    vector<string> nse_keys, bse_keys;
    for (const auto& pair_key : instrument_keys) {
        nse_keys.push_back(pair_key.first);
        bse_keys.push_back(pair_key.second);
    }
    
    signal(SIGINT, signalHandler); // Handle Ctrl+C to stop the loop

    while (keep_running) {
    	cout << endl;
        cout << "----------------------------------------------------------------------------Start---------------------------------------------------------------------------------------" << endl;
        
        int num_chunks = (nse_keys.size() + MAX_KEYS_PER_REQUEST - 1) / MAX_KEYS_PER_REQUEST;
        vector<pthread_t> threads(num_chunks * 2); // Two threads per chunk (NSE & BSE)

        pthread_mutex_init(&mtx, nullptr);

        // Launch threads for fetching data
        for (int i = 0; i < num_chunks; i++) {
            int start_idx = i * MAX_KEYS_PER_REQUEST;
            int end_idx = min(start_idx + MAX_KEYS_PER_REQUEST, (int)nse_keys.size());

            vector<string> nse_chunk(nse_keys.begin() + start_idx, nse_keys.begin() + end_idx);
            vector<string> bse_chunk(bse_keys.begin() + start_idx, bse_keys.begin() + end_idx);

            ThreadArgs* args_nse = new ThreadArgs{nse_chunk, true};
            ThreadArgs* args_bse = new ThreadArgs{bse_chunk, false};

            pthread_create(&threads[i * 2], nullptr, fetchMarketDataThread, args_nse);
            pthread_create(&threads[i * 2 + 1], nullptr, fetchMarketDataThread, args_bse);
        }

        // Wait for all fetching threads
        for (pthread_t& thread : threads) {
            pthread_join(thread, nullptr);
        }

        pthread_mutex_destroy(&mtx);

        json market_data = mergeMarketData(market_data_nse, market_data_bse);

        int num_threads = 5;
        int chunk_size = market_data.size() / num_threads;
        vector<pthread_t> simulation_threads(num_threads);
        vector<ThreadData> thread_data(num_threads);

        pthread_mutex_init(&print_mutex, nullptr);

        for (int i = 0; i < num_threads; ++i) {
            thread_data[i] = {&market_data, i * chunk_size, (i == num_threads - 1) ? (int)market_data.size() : (i + 1) * chunk_size};
            pthread_create(&simulation_threads[i], nullptr, simulateArbitrage, &thread_data[i]);
        }

        for (pthread_t& thread : simulation_threads) {
            pthread_join(thread, nullptr);
        }

        pthread_mutex_destroy(&print_mutex);
        
        cout << "-----------------------------------------------------------------------------End----------------------------------------------------------------------------------------" << endl;
        cout << "-----------------------------------------------Next Simulation after 20 seconds. Press \"ctrl+c\" to stop the simulation--------------------------------------------------" << endl;

        sleep(20);
    }

    cout << "Stopped arbitrage simulation.\n";
    return 0;
}
