import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from openpyxl import Workbook
from openpyxl.drawing.image import Image
from io import BytesIO

# Sample input data
log2_size = np.arange(10, 21)  # Log2 sizes from 10 to 20
size = 2**log2_size  # Actual sizes in KB
miss_rate = np.array([0.1, 0.2, 0.33, 0.4, 0.5, 0.6, 0.7, 0.75, 0.8, 0.9, 0.95])[:len(log2_size)]
associativities = [1, 2, 4, 8, 16, 32]  # Different associativities

# Create a Pandas DataFrame to store data for Excel
df = pd.DataFrame({'Log2(Size)': log2_size, 'Size (KB)': size})

# Create a new Excel file
wb = Workbook()
ws = wb.active
ws.title = "Miss Rate Graphs"

# Function to generate graphs
def create_graph(associativity):
    plt.figure(figsize=(8, 6))
    plt.plot(log2_size, miss_rate, marker='o', label=f'Associativity: {associativity}')
    plt.xlabel('Log2(Size)')
    plt.ylabel('Miss Rate')
    plt.title(f'Log2(Size) vs Miss Rate for Associativity {associativity}')
    plt.legend()
    plt.grid(True)

    # Save plot to BytesIO object
    imgdata = BytesIO()
    plt.savefig(imgdata, format='png')
    plt.close()

    # Insert image into the Excel file
    img = Image(imgdata)
    img.anchor = ws.cell(row=1, column=associativities.index(associativity) + 1).coordinate  # Anchor the image
    ws.add_image(img)

# Generate a graph for each associativity
for associativity in associativities:
    create_graph(associativity)

# Save Excel file
wb.save('miss_rate_graphs.xlsx')
