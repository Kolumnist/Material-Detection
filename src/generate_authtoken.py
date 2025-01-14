from google.auth.transport.requests import Request
from google.oauth2 import service_account

SERVICE_ACCOUNT_FILE = "C:\dev\personal\Material Detection\subscriber.json"

SCOPES = ["https://www.googleapis.com/auth/cloud-platform"]

# Create credentials from the service account
credentials = service_account.Credentials.from_service_account_file(
    SERVICE_ACCOUNT_FILE, scopes=SCOPES)

# Refresh the credentials to obtain an access token
credentials.refresh(Request())

# Print the access token
print("Access Token:", credentials.token)
