import csv

filename = 'data/data.tsv'
column_number = 2 # Replace with the index of the column you want to analyze (starting from 0)

with open(filename, 'r') as tsvfile:
    reader = csv.reader(tsvfile, delimiter='\t')
    max_value = None
    for row in reader:
        try:
            value = float(row[column_number])
            if max_value is None or value > max_value:
                max_value = value
        except ValueError:
            pass  # Ignore non-numeric values in the column
    if max_value is not None:
        print(f"The largest number in column {column_number} is: {max_value}")
    else:
        print(f"No valid numbers found in column {column_number}")