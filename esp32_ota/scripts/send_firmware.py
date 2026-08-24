import sys
import requests

def send_firmware(bin_path, esp32_url):
    print(f"Envoi du fichier {bin_path} vers {esp32_url}...")
    try:
        with open(bin_path, 'rb') as f:
            # Envoi au format multipart/form-data (identique a curl -F "firmware=@...")
            files = {'firmware': (bin_path, f, 'application/octet-stream')}
            r = requests.post(esp32_url, files=files, timeout=30)
            
        print(f"Reponse ESP32 : {r.status_code} - {r.text}")
        return r.status_code == 200

    except Exception as e:
        print(f"Erreur lors de l'envoi : {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 send_firmware.py <path_to_bin> <esp32_url>")
        sys.exit(1)
    
    bin_path = sys.argv[1]
    esp32_url = sys.argv[2]
    success = send_firmware(bin_path, esp32_url)
    sys.exit(0 if success else 1)
