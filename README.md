Arbitrage Trading Simulation

📌 Overview

This project implements an Arbitrage Trading Simulation that detects price discrepancies between NSE (National Stock Exchange) and BSE (Bombay Stock Exchange) and simulates buy-sell orders accordingly. The system fetches real-time stock data and highlights arbitrage opportunities while considering all associated charges.

🚀 Features

Real-time Data Fetching: Fetches bid-ask prices for 220 stocks from both NSE and BSE using the Upstox Uplink API.

Parallel Processing with Pthreads:

One thread fetches market data.

Multiple threads (e.g., 4) handle different subsets of stocks for arbitrage detection.

Synchronization Using Mutex: Ensures thread-safe data access while processing stock prices.

Arbitrage Opportunity Detection: Calculates potential profits after accounting for transaction costs.

Efficient Execution: Uses cURL in C++ to interact with the API and process data efficiently.

🛠️ Technologies Used

C++ (Primary language)

Pthreads (For parallel processing)

Mutex (For synchronization)

cURL (For API communication)

Upstox Uplink API (For real-time stock data)

🔧 Setup & Usage

Clone the repository:

git clone https://github.com/your-username/arbitrage-trading.git
cd arbitrage-trading

Compile the program:

g++ -pthread -o arbitrage_simulator main.cpp -lcurl

Run the program:

./arbitrage_simulator

📈 Arbitrage Strategy

Fetch the bid-ask prices of stocks listed on both NSE & BSE.

Detect stocks where price differences exist after considering charges.

Simulate a trade by virtually buying at the lower-priced exchange and selling at the higher-priced one.

Log the arbitrage opportunities for analysis.

📌 Future Improvements

Implementing actual trade execution instead of simulation.

Enhancing performance with optimized data structures.

Adding GUI for visualization of arbitrage opportunities.

🤝 Contributions

Feel free to fork, modify, and contribute to this project! Open an issue if you find any bugs or have feature suggestions.

📜 License

This project is licensed under the MIT License.
