import smtplib
import sqlite3
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

def send_bulk_reminders():
    # --- 1. SETTINGS ---
    # For Gmail: "smtp.gmail.com" | For Outlook/Office365: "smtp.office365.com"
    SMTP_SERVER = "smtp.gmail.com" 
    SMTP_PORT = 587
    MY_EMAIL = "32211305@gendejesus.edu.ph"
    MY_APP_PASSWORD = "kptr gvpd ejkx vutu" # Your 16-character App Password

    try:
        # 2. Connect to Database
        conn = sqlite3.connect("local_data.db")
        cursor = conn.cursor()
        cursor.execute("SELECT Email, StudentName, BookBorrowed FROM librarylog WHERE Status = 'Not Returned'")
        rows = cursor.fetchall()

        if not rows:
            print("No overdue books found.")
            conn.close()
            return

        # 3. Start the Email Server Connection
        server = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
        server.starttls()  # Encrypts the connection
        server.login(MY_EMAIL, MY_APP_PASSWORD)

        for email, name, book in rows:
            if not email: continue

            # Create the Email message
            msg = MIMEMultipart()
            msg['From'] = f"JHS Library <{MY_EMAIL}>"
            msg['To'] = email
            msg['Subject'] = "Weekly Library Reminder"

            body = f"Hi {name},<br><br>Our records show you still have <strong>{book}</strong>. Please return it to the JHS Library as soon as possible."
            msg.attach(MIMEText(body, 'html'))

            try:
                server.send_message(msg)
                print(f"✅ Sent to {email}")
            except Exception as e:
                print(f"❌ Failed for {email}: {e}")

        # 4. Cleanup
        server.quit()
        conn.close()
        print("\nAll reminders processed.")

    except Exception as e:
        print(f"Connection error: {e}")

if __name__ == "__main__":
    send_bulk_reminders()
