import pandas as pd
import csv

# Load the CSV files
nse_df = pd.read_csv('nse.csv')
bse_df = pd.read_csv('bse.csv')

# Extract the numeric part after "NSE_EQ|" and "BSE_EQ|"
nse_df['key'] = nse_df['instrument_key'].str.replace(r'NSE_EQ\|', '', regex=True)
bse_df['key'] = bse_df['instrument_key'].str.replace(r'BSE_EQ\|', '', regex=True)

# Merge the dataframes on the extracted key
merged_df = pd.merge(nse_df, bse_df, on='key', suffixes=('_nse', '_bse'))

# Filter rows where tradingsymbol_nse == tradingsymbol_bse
filtered_df = merged_df[merged_df['tradingsymbol_nse'] == merged_df['tradingsymbol_bse']]

# Select relevant columns
result_df = filtered_df[['tradingsymbol_nse', 'instrument_key_nse', 'instrument_key_bse']]

# Save the final data to a new CSV file
result_df.to_csv('common_equity.csv', index=False, quoting=csv.QUOTE_ALL)

print("CSV file 'common_equity.csv' has been created successfully with matching trading symbols!")

