import smtplib
import sqlite3
import os
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

def send_bulk_reminders():
    # SETTINGS
    MY_EMAIL = "32211305@gendejesus.edu.ph"
    MY_APP_PASSWORD = "kptr gvpd ejkx vutu"
    
    # Get the folder where THIS script is located
    base_path = os.path.dirname(os.path.abspath(__file__))
    db_path = os.path.join(base_path, "local_data.db")

    print(f"Connecting to database at: {db_path}")

    try:
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        # Use LIKE to be safe with spaces/casing
        cursor.execute("SELECT Email, StudentName, BookBorrowed FROM librarylog WHERE Status LIKE '%Not Returned%'")
        rows = cursor.fetchall()

        if not rows:
            print("Finished: No students found with 'Not Returned' status.")
            return

        print(f"Found {len(rows)} emails to send. Connecting to Gmail...")

        server = smtplib.SMTP("smtp.gmail.com", 587)
        server.starttls()
        server.login(MY_EMAIL, MY_APP_PASSWORD)

        for email, name, book in rows:
            if not email: continue
            msg = MIMEMultipart()
            msg['From'] = f"JHS Library <{MY_EMAIL}>"
            msg['To'] = email
            msg['Subject'] = "Weekly Library Reminder"
            body = f"Hi {name},<br><br>Please return <strong>{book}</strong> to the JHS Library."
            msg.attach(MIMEText(body, 'html'))
            
            server.send_message(msg)
            print(f"✅ Successfully sent to: {email}")

        server.quit()
        conn.close()
        print("Done!")

    except Exception as e:
        print(f"FATAL ERROR: {str(e)}")

if __name__ == "__main__":
    send_bulk_reminders()