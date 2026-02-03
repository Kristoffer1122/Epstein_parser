import glob
import os
import shutil
import threading
import time

import pypdf

TARGET_FLAG = False

# FOLDER_PATH = "./data/"
FOLDER_PATH = glob.glob("./VOL00010/IMAGES/*/")
# FOLDER_PATH = glob.glob("./data/")
PROCESSED_FOLDER = "./processed/"
TARGETS = []

while TARGET_FLAG is False:
    target_input = input("Enter target string to search for in PDFs: ")
    TARGETS.append(target_input.lower())
    TARGET_FLAG = input("Add more targets? (y/n): ").lower() != "y"

# Ensure the processed folder exists
if not os.path.exists(PROCESSED_FOLDER):
    os.makedirs(PROCESSED_FOLDER)

matches = []


# Function to parse PDF and search for target strings
def parse_PDF(file_path):
    match_count = 0
    try:
        print(f"Processing file: {file_path}", flush=True)
        reader = pypdf.PdfReader(file_path)

        # Check each page for target strings (case-insensitive)
        for page_number, page in enumerate(reader.pages, start=1):
            text = page.extract_text()

            if not text:
                print(
                    f"Warning: No text found on page {page_number} of {file_path}",
                    flush=True,
                )
                continue  # Skip empty pages

            # Convert to lowercase for consistent matching
            text = text.lower()

            for target in TARGETS:
                if target in text:
                    if file_path not in matches:  # Avoid duplicate entries
                        matches.append(file_path)
                        # Copy matching PDF to the processed folder
                        shutil.copy2(file_path, PROCESSED_FOLDER)
                    match_count += 1

    except Exception as e:
        print(f"Error processing {file_path}: {e}", flush=True)

    print(f"Matches in {file_path}: {match_count}", flush=True)


# Process each folder and its PDF files
threads = []

for folder in FOLDER_PATH:
    # Find all PDFs within the current folder
    pdf_files = glob.glob(os.path.join(folder, "*.pdf"))

    print(f"Found {len(pdf_files)} PDF files in {folder}", flush=True)

    for pdf_file in pdf_files:
        thread = threading.Thread(target=parse_PDF, args=(pdf_file,))
        threads.append(thread)
        thread.start()

# Wait for all threads to finish
for thread in threads:
    thread.join()

print("Processing complete. Total matches:", len(matches))
print("Matched files:", matches)
