import pandas as pd
import csv

# Load the CSV files
nse_df = pd.read_csv('NSE.csv')
bse_df = pd.read_csv('BSE.csv')

# Filter NSE data where instrumentkey starts with "NSE_EQ|"
nse_filtered = nse_df[nse_df['instrument_key'].str.startswith('NSE_EQ|')]

# Filter BSE data where instrumentkey starts with "BSE_EQ|"
bse_filtered = bse_df[bse_df['instrument_key'].str.startswith('BSE_EQ|')]

nse_filtered.to_csv('nse.csv', index=False, quoting=csv.QUOTE_ALL)
bse_filtered.to_csv('bse.csv', index=False, quoting=csv.QUOTE_ALL)
