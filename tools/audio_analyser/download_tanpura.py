import os
import requests

# Dictionary mapping URLs to a descriptive filename
tanpura_samples = {
    # Sa-Pa Tuning (1-5)
    "SaPa_A.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/a-tanpura-thick.mp3",
    "SaPa_Asharp_Bb.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/ab-tanpura-thick.mp3",
    "SaPa_B.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/b-tanpura-thick.mp3",
    "SaPa_C.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/c-tanpura-thick.mp3",
    "SaPa_Csharp_Db.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/db-tanpura-thick.mp3",
    "SaPa_D.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/d-tanpura-thick.mp3",
    "SaPa_Dsharp_Eb.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/eb-tanpura-thick.mp3",
    "SaPa_E.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/e-tanpura-thick.mp3",
    "SaPa_F.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/f-tanpura-thick.mp3",
    "SaPa_Fsharp_Gb.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/gb-tanpura-thick.mp3",
    "SaPa_G.mp3": "https://ragajunglism.org/wp-content/uploads/2020/03/g-tanpura-thick.mp3",
    "SaPa_Gsharp_Ab.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/Ab-Tanpura-B5-SaMa-2021.mp3",
    
    # Sa-Ma Tuning (1-4)
    "SaMa_A.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/A-Tanpura-B5-SaMa-2021.mp3",
    "SaMa_C.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/C-Tanpura-B5-SaMa-2021.mp3",
    "SaMa_Csharp_Db.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/Db-Tanpura-B5-SaMa-2021.mp3",
    "SaMa_D.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/D-Tanpura-B5-SaMa-2021.mp3",
    "SaMa_G.mp3": "https://ragajunglism.org/wp-content/uploads/2021/02/G-Tanpura-B5-SaMa-2021.mp3"
}

def download_tanpuras(data_dict):
    # Create a directory for the samples
    folder = "Tanpura_Samples_DSP"
    if not os.path.exists(folder):
        os.makedirs(folder)
    
    print(f"Starting download into folder: {folder}...")

    for filename, url in data_dict.items():
        filepath = os.path.join(folder, filename)
        try:
            print(f"Downloading {filename}...")
            response = requests.get(url, stream=True, timeout=30)
            response.raise_for_status() # Check for HTTP errors
            
            with open(filepath, 'wb') as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            print(f"Successfully saved {filename}")
        except Exception as e:
            print(f"Failed to download {filename}: {e}")

if __name__ == "__main__":
    download_tanpuras(tanpura_samples)
    print("\nAll downloads complete.")