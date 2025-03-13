# Arbitrage Trading Simulation


## 📌 Overview

This project implements an Arbitrage Trading Simulation that detects price discrepancies between NSE (National Stock Exchange) and BSE (Bombay Stock Exchange) and simulates buy-sell orders accordingly. The system fetches real-time stock data and highlights arbitrage opportunities while considering all associated charges.


## 🚀 Features

📌 Real-time Data Fetching: Fetches last trade price (ltp) for 2000+ stocks from both NSE and BSE using the Upstox Uplink API.

📌 Parallel Processing with Pthreads: Multiple threads fetch market data for different subsets of stocks, Multiple threads handle different subsets of stocks for arbitrage detection.

📌 Synchronization Using Mutex: Ensures thread-safe data access while processing stock prices.

📌 Arbitrage Opportunity Detection: Calculates potential profits after accounting for transaction costs.

📌 Efficient Execution: Uses cURL in C++ to interact with the API and process data efficiently.


## 🛠️ Technologies Used

📌 C++ (Primary language)

📌 Pthreads (For parallel processing)

📌 Mutex (For synchronization)

📌 cURL (For API communication)

📌 Upstox Uplink API (For real-time stock data)


## 🔧 Setup & Usage

1. Clone the repository
```bash
git clone https://github.com/jatinjindal1203/Arbitrage-Trading-Simulator.git
cd Arbitrage-Trading-Simulator
```
2. Login to <https://account.upstox.com/developer/apps> and create an app then you will get your API_KEY and API_SECRET.

3. Store the API_KEY, API_SECRET and REDIRECT_URI in api_keys.txt as
```txt
API_KEY = <YPUR_API_KEY>
API_SECRET = <YOUR_API_SECRET>
REDIRECT_URI = <YOUR_REDIRECT_URI>
```
REDIRECT_URI is same as you have entered while create an app.

4. Install the dependencies:
```bash
sudo apt update
sudo apt install -y curl libcurl4-openssl-dev nlohmann-json3-dev
```
5. Compile the program:
```bash
g++ -o access_token access_token.cpp -lcurl -ljsoncpp -std=c++17
g++ -o arbitrage arbitrage.cpp -pthread -lcurl -ljsoncpp -std=c++17
```

6. Run the file "access_token":
```bash
./access_token
```
You will get an url, login your upstox account here then you will be redirected to a url, extract the last six characters of the redirected url which is your code and enter this code as a input to the running program.

7. Run the file "arbitrage":
```bash
./arbitrage
```
First Enter your capital amount, then Enter minimum profit per trade, and see the arbitrage trade opportunities.


## 📈 Arbitrage Strategy

1. Fetch the last trade price of stocks listed on both NSE & BSE.
2. Detect stocks where price differences exist after considering charges.
3. Simulate a trade by virtually selling at the higher-priced one and buying at the lower-priced exchange.


## 📌 Future Improvements

1. Implementing actual trade execution instead of simulation.
2. Enhancing performance with optimized data structures.
3. Adding GUI for visualization of arbitrage opportunities.


## 📈 One Catch

📌 In India you can take an arbitrage trade only those stocks which are present in your portfolio.


## 🤝 Contributions

Feel free to fork, modify, and contribute to this project! Open an issue if you find any bugs or have feature suggestions.


## 📜 License

This project is licensed under the MIT License.
