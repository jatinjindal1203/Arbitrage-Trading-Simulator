#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

string API_KEY, API_SECRET, REDIRECT_URI;

// Function to read API credentials from a file with "KEY = VALUE" format
void readAPIKeys(const string& filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open " << filename << endl;
        exit(1);
    }

    string line, key, value;
    while (getline(file, line)) {
        stringstream ss(line);
        if (getline(ss, key, '=') && getline(ss, value)) {
            key.erase(remove_if(key.begin(), key.end(), ::isspace), key.end());
            value.erase(remove_if(value.begin(), value.end(), ::isspace), value.end());

            if (key == "API_KEY") API_KEY = value;
            else if (key == "API_SECRET") API_SECRET = value;
            else if (key == "REDIRECT_URI") REDIRECT_URI = value;
        }
    }
    file.close();
}

// Function to URL encode a string
string urlEncode(const string &value) {
    CURL *curl = curl_easy_init();
    if (!curl) return "";

    char *encoded = curl_easy_escape(curl, value.c_str(), value.length());
    string result(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

// Function to handle HTTP responses
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

// Function to get access token
string getAccessToken(const string& auth_code) {
    CURL* curl;
    CURLcode res;
    string response_data;

    curl = curl_easy_init();
    if(curl) {
        string url = "https://api.upstox.com/v2/login/authorization/token";
        string post_fields = "code=" + auth_code + "&client_id=" + API_KEY + "&client_secret=" + API_SECRET + 
                             "&redirect_uri=" + REDIRECT_URI + "&grant_type=authorization_code";

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    try {
        json response_json = json::parse(response_data);
        return response_json["access_token"].get<string>();
    } catch (...) {
        cerr << "Error parsing response. Response: " << response_data << endl;
        return "";
    }
}

// Function to save access token to a file
void saveAccessToken(const string& token) {
    ofstream file("access_token.txt");
    if (!file) {
        cerr << "Error: Unable to save access token" << endl;
        return;
    }
    file << token;
    file.close();
}

int main() {
    // Read API credentials
    readAPIKeys("api_keys.txt");
    
   cout << REDIRECT_URI << endl;

    string encodedUrl = urlEncode(REDIRECT_URI);
    string uri = "https://api-v2.upstox.com/login/authorization/dialog?"
                 "response_type=code&client_id=" + API_KEY +
                 "&redirect_uri=" + encodedUrl;

    cout << "Login to Upstox at URI: " << uri << endl;

    string auth_code;
    cout << "Enter the authorization code from Upstox (Last 6 digits of the URI on which you are redirected after login): ";
    cin >> auth_code;

    string access_token = getAccessToken(auth_code);
    
    if (!access_token.empty()) {
        saveAccessToken(access_token);
        cout << "Access token saved to access_token.txt" << endl;
    } else {
        cerr << "Failed to retrieve access token." << endl;
    }

    return 0;
}
