import glob
import os
import shutil
import time
from multiprocessing import Manager, Pool

import pypdf

# Gathering PDF folder paths and defining constants
FOLDER_PATH = glob.glob("./VOL00010/IMAGES/*/")  # Update your folder path here
PROCESSED_FOLDER = ""
TARGETS = []

# Collecting target strings in one loop
while True:
    target_input = input("Enter target string to search for in PDFs: ").strip().lower()
    TARGETS.append(target_input)
    if input("Add more targets? (y/n): ").strip().lower() != "y":
        break

# Choose where to save output files
while True:
    folder_target = input("Enter Folder to save data in: ").strip()
    PROCESSED_FOLDER = f"./{folder_target}/"
    break

# Print targets for confirmation
for target in TARGETS:
    print(f"Target added: {target}", flush=True)

# Ensure the processed folder exists
os.makedirs(PROCESSED_FOLDER, exist_ok=True)


def parse_PDF(file_path, targets, matches):
    """Function to parse a single PDF and search for target strings."""
    match_count = 0

    time_start = time.time()
    try:
        # print(f"Processing file: {file_path}", flush=True)
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

            for target in targets:
                if target in text:
                    if file_path not in matches:  # Avoid duplicate entries
                        matches.append(file_path)
                        print(f"Match found for '{target}' in {file_path}", flush=True)

                        # Copy matching PDF to the processed folder
                        shutil.copy2(file_path, PROCESSED_FOLDER)
                    match_count += 1

    except Exception as e:
        print(f"Error processing {file_path}: {e}", flush=True)
    time_end = time.time()
    print(
        f"Time taken for {file_path}: {time_end - time_start:.2f} seconds", flush=True
    )

    # print(f"Matches in {file_path}: {match_count}", flush=True)


def worker_task(file_path, targets, matches):
    """Worker task to process a single PDF file."""
    parse_PDF(file_path, targets, matches)


if __name__ == "__main__":
    with Manager() as manager:
        matches = manager.list()  # Shared list to store match results

        # Gather all files across folders
        pdf_files = []
        for folder in FOLDER_PATH:
            pdf_files.extend(glob.glob(os.path.join(folder, "*.pdf")))

        print(f"Found {len(pdf_files)} PDF files.", flush=True)

        # Create a worker pool with multiple processes
        with Pool(processes=os.cpu_count()) as pool:
            pool.starmap(
                worker_task, [(pdf_file, TARGETS, matches) for pdf_file in pdf_files]
            )

        print("Processing complete. Total matches:", len(matches))
        print("Matched files:", list(matches))
