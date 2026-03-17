// library app.cpp : Defines the entry point for the application.
//

#include <windowsx.h>
#include <cstdio>
#include "framework.h"
#include "library app.h"
#include <commctrl.h>
#include "sqlite3.h"
#include <ctime>
#include <shellapi.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

sqlite3* db = nullptr;
sqlite3* db_test = nullptr;
sqlite3* db_book = nullptr;

void RemoveAllSpacesW(wchar_t* s) {
    if (!s) return;
    wchar_t* d = s;
    do {
        while (iswspace(*s)) {
            s++;
        }
    } while (*d++ = *s++);
}

// Helper function to execute SQL
void ExecuteSQL(const char* sql) {
    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, sql, NULL, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        MessageBoxA(NULL, errorMessage, "SQLite Error", MB_ICONERROR);
        sqlite3_free(errorMessage);
    }
}

void RefreshListView(HWND hWndListView, sqlite3* db_local, bool scroll) {
    // 1. Clear existing items
    ListView_DeleteAllItems(hWndListView);

    // 2. Prepare the SELECT query
    const char* query = "SELECT ROW_NUMBER() OVER (ORDER BY ID ASC) AS RefNum, StudentID, StudentName, Year_Section, Email, BookBorrowed, ReturnDate , Status FROM librarylog;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_local, query, -1, &stmt, NULL) == SQLITE_OK) {
        int index = 0;

        // 3. Loop through each row in the database
        while (sqlite3_step(stmt) == SQLITE_ROW) {
			const unsigned char* rowID = sqlite3_column_text(stmt, 0);
            const unsigned char* id = sqlite3_column_text(stmt, 1);
            const unsigned char* name = sqlite3_column_text(stmt, 2);
            const unsigned char* section = sqlite3_column_text(stmt, 3);
            const unsigned char* email = sqlite3_column_text(stmt, 4);
            const unsigned char* book = sqlite3_column_text(stmt, 5);
            int date = sqlite3_column_int(stmt, 6);
			const unsigned char* status = sqlite3_column_text(stmt, 7);

            // 4. Insert into ListView
            LVITEM lvItem = { 0 };
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = index;

            // Column 0: Student ID (Convert UTF-8 to Wide String)
            wchar_t wText[256];
            swprintf_s(wText, L"%hs", rowID);
            lvItem.pszText = wText;
            ListView_InsertItem(hWndListView, &lvItem);

            swprintf_s(wText, L"%hs", id);
            ListView_SetItemText(hWndListView, index, 1, wText);

            // Column 1: Name
            swprintf_s(wText, L"%hs", name);
            ListView_SetItemText(hWndListView, index, 2, wText);

            // Column 2: Section
            swprintf_s(wText, L"%hs", section);
            ListView_SetItemText(hWndListView, index, 3, wText);

            swprintf_s(wText, L"%hs", email);
            ListView_SetItemText(hWndListView, index, 4, wText);

            swprintf_s(wText, L"%hs", book);
            ListView_SetItemText(hWndListView, index, 5, wText);

            swprintf_s(wText, L"%hs", status);
            ListView_SetItemText(hWndListView, index, 7, wText);

        

            index++;

            if (scroll) {
                ListView_EnsureVisible(hWndListView, index - 1, FALSE);
                // Note: ListView_EnsureVisible is more reliable than ListView_Scroll
            }
        }
        sqlite3_finalize(stmt);
    }
}

void CheckAndSendWeeklyEmails() {
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);

    if (ltm.tm_wday == 4) { // Thursday
        char todayDate[20];
        strftime(todayDate, sizeof(todayDate), "%Y-%m-%d", &ltm);

        // 1. Check date
        sqlite3_stmt* stmt;
        const char* checkSql = "SELECT value FROM app_settings WHERE key = 'last_email_date';";

        if (sqlite3_prepare_v2(db, checkSql, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                const char* lastDate = (const char*)sqlite3_column_text(stmt, 0);

                if (strcmp(lastDate, todayDate) != 0) {

              
                    sqlite3_stmt* dataStmt;
                    const char* dataCheck = "SELECT COUNT(*) FROM librarylog WHERE Status = 'Not Returned';";
                    int count = 0;
                    if (sqlite3_prepare_v2(db, dataCheck, -1, &dataStmt, NULL) == SQLITE_OK) {
                        if (sqlite3_step(dataStmt) == SQLITE_ROW) {
                            count = sqlite3_column_int(dataStmt, 0);
                        }
                        sqlite3_finalize(dataStmt);
                    }

                    if (count > 0) { // Only send if someone hasn't returned a book
                        ShellExecuteA(NULL, "open", "python.exe", "email_script.py", NULL, SW_HIDE);
                        MessageBox(NULL, L"Weekly reminder sent!", L"Email System", MB_OK);

                        char updateSql[128];
                        sprintf_s(updateSql, "UPDATE app_settings SET value = '%s' WHERE key = 'last_email_date';", todayDate);
                        sqlite3_exec(db, updateSql, NULL, NULL, NULL);
                    }
                }
            }
            sqlite3_finalize(stmt);
        }
    }
}


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MyDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DeleteDialogProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    
    // 1. Try to open the Test DB
    int rc = sqlite3_open("test_data.db", &db_test);

    if (rc != SQLITE_OK) {
        // ONLY show the error here
        const char* err = sqlite3_errmsg(db_test);
        wchar_t wmsg[256];
        swprintf_s(wmsg, L"Test DB Open Failed: %hs", err);
        MessageBox(NULL, wmsg, L"Critical Error", MB_ICONERROR);
        return 0;
    }

    // 2. SUCCESS PATH: Now run the setup because the DB is officially open
    const char* setup_test =
        "CREATE TABLE IF NOT EXISTS students (StudentID INTEGER, Name TEXT, Section TEXT, Email TEXT, Status TEXT);"
        "INSERT OR IGNORE INTO students VALUES (123456, 'Vince Michael Dela Cruz', '12-Galileo', '123456@gende.com', 'Not Returned');"
        "INSERT OR IGNORE INTO students VALUES (32211305, 'Ron Devuaghn Lemence', '12-Galileo', '32211305@gendejesus.edu.ph', 'Not Returned');"
        "INSERT OR IGNORE INTO students VALUES (32310232, 'Nate Danniel Pre', '12-Galileo', '32310232@gendejesus.edu.ph', 'Not Returned');"
        "INSERT OR IGNORE INTO students VALUES (11111111, 'Nate Danniel Pre', '12-Galileo', 'ansesskahz@gmail.com', 'Not Returned');"

        "CREATE TABLE IF NOT EXISTS books (BookID INTEGER, BookName TEXT);";

    sqlite3_exec(db_test, setup_test, NULL, NULL, NULL);
    sqlite3_close(db_test); // Close it so it's ready to be ATTACHED later



    // Open or create the local database file
    // 1. Open the DB
    if (sqlite3_open("local_data.db", &db) == SQLITE_OK) {
        sqlite3_busy_timeout(db, 5000); // Set timeout first

        const char* schema =
            "CREATE TABLE IF NOT EXISTS librarylog ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
            "StudentID INTEGER, "
            "StudentName TEXT, "
            "Year_Section TEXT,"
            "Email TEXT,"
            "BookBorrowed TEXT,"
            "ReturnDate TEXT,"
            "Status TEXT DEFAULT 'Not Returned'"
            ");"

            "CREATE TABLE IF NOT EXISTS app_settings (key TEXT PRIMARY KEY, value TEXT);"
            "INSERT OR IGNORE INTO app_settings VALUES('last_email_date', '0');";
        if (sqlite3_exec(db, schema, NULL, NULL, NULL) == SQLITE_OK) {

            // 3. Optional: Verify a table actually exists before calling weekly check
            sqlite3_stmt* check;
            if (sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table' AND name='librarylog';", -1, &check, NULL) == SQLITE_OK) {
                if (sqlite3_step(check) == SQLITE_ROW) {
                    CheckAndSendWeeklyEmails(); // Only run if tables are confirmed
                }
                sqlite3_finalize(check);
            }
        }
    }
    

    InitCommonControls();
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LIBRARYAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LIBRARYAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LIBRARYAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LIBRARYAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND libraryDataGrid;
HWND comboBox;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   RECT rc;
   GetClientRect(hWnd, &rc);
   int winHeight = rc.bottom; // This is the total height
   int winWidth = rc.right;

   int startY = 5;
   int startHeight = winHeight - startY - 10; // 10 is a bottom margin

    libraryDataGrid = CreateWindowExW(0, WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
        5, startY, 800, startHeight,
		hWnd, (HMENU)1, hInstance, nullptr);

    comboBox = CreateWindowExW(0, WC_COMBOBOX, L"",
        WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
        820, startY, 200, 300,
		libraryDataGrid, (HMENU)3, hInstance, nullptr);

	ListView_SetExtendedListViewStyle(libraryDataGrid, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMN libraryDataGridColumns;
        libraryDataGridColumns.mask = LVCF_TEXT | LVCF_WIDTH;
	    libraryDataGridColumns.cx = 100;

        libraryDataGridColumns.pszText = (LPWSTR) L"Row ID";
        ListView_InsertColumn(libraryDataGrid, 0, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Student ID";
        ListView_InsertColumn(libraryDataGrid, 1, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Name";
        ListView_InsertColumn(libraryDataGrid, 2, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Year & Section";
        ListView_InsertColumn(libraryDataGrid, 3, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR)L"Email Adress";
        ListView_InsertColumn(libraryDataGrid, 4, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Book Borrowed";
	    ListView_InsertColumn(libraryDataGrid, 5, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Return Date";
	    ListView_InsertColumn(libraryDataGrid, 6, &libraryDataGridColumns);
		libraryDataGridColumns.pszText = (LPWSTR) L"Status";
        ListView_InsertColumn(libraryDataGrid, 7, &libraryDataGridColumns);


		// Insert sample data
        RefreshListView(libraryDataGrid, db, true);
    
        
   ShowWindow(hWnd, SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        int winWidth = LOWORD(lParam);
        int winHeight = HIWORD(lParam);

        HWND hGrid = GetDlgItem(hWnd, 1);
        if (hGrid)
        {
            int padding = 5;
            int topSectionHeight = 5;

            // MATH: Calculate 80% width and subtract padding
            int gridWidth = (int)(winWidth * 0.85) - padding;

            // MATH: Keep your dynamic height calculation
            int gridHeight = winHeight - topSectionHeight - padding;

            // Apply the size
            // We keep it at X=padding so the 20% gap stays on the RIGHT
            SetWindowPos(hGrid, NULL, padding, topSectionHeight, gridWidth, gridHeight, SWP_NOZORDER);

            ListView_SetColumnWidth(hGrid, 0, 75);
            ListView_SetColumnWidth(hGrid, 1, 100);
            ListView_SetColumnWidth(hGrid, 2, 400);
            ListView_SetColumnWidth(hGrid, 3, 150);
            ListView_SetColumnWidth(hGrid, 4, 300);
            ListView_SetColumnWidth(hGrid, 5, 300);
            ListView_SetColumnWidth(hGrid, 6, 200);
            ListView_SetColumnWidth(hGrid, 7, LVSCW_AUTOSIZE_USEHEADER);
        }
    }
    break;
    case WM_INITDIALOG:
    {
        /* // Add items to the dropdown
         SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)L"First Item");
         SendMessage(comboBox, CB_ADDSTRING, 0, (LPARAM)L"Second Item");

         // Optional: Set the default selection to the first item
         SendMessage(comboBox, CB_SETCURSEL, 0, 0);

         return TRUE;*/
    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);

        switch (wmId)
        {
        case 2:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, MyDialogProc);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case 4:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, DeleteDialogProc);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case 1001: // Mark as Returned
        case 1002: // Mark as Not Returned
        {
            int selectedIndex = ListView_GetNextItem(libraryDataGrid, -1, LVNI_SELECTED);
            if (selectedIndex != -1) {
                // 1. Get the UNIQUE ID from Column 0 (the Row ID)
                wchar_t szRowID[256];
                ListView_GetItemText(libraryDataGrid, selectedIndex, 0, szRowID, 256);

                const char* newStatus = (wmId == 1001) ? "Returned" : "Not Returned";
                const char* newDate = (wmId == 1001) ? "datetime('now', 'localtime')" : "NULL";

                // 2. IMPORTANT: Change StudentID to ID in the WHERE clause
                char sql[512];
                sprintf_s(sql, "UPDATE librarylog SET Status = '%s', ReturnDate = %s WHERE ID = %ls;",
                    newStatus, newDate, szRowID);

                if (sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) {
                    RefreshListView(libraryDataGrid, db, false);
                }
            }
            break;
        }
        break; // This break is OUTSIDE the curly braces

        case 1003: // Delete Record
        {
            int selectedIndex = ListView_GetNextItem(libraryDataGrid, -1, LVNI_SELECTED);
            if (selectedIndex != -1) {
                if (MessageBox(hWnd, L"Are you sure you want to delete this specific record?", L"Confirm", MB_YESNO | MB_ICONWARNING) == IDYES) {
                    // Get the ID from Column 0
                    wchar_t szRowID[256];
                    ListView_GetItemText(libraryDataGrid, selectedIndex, 0, szRowID, 256);

                    char sql[512];
                    sprintf_s(sql, "DELETE FROM librarylog WHERE ID = %ls;", szRowID);

                    sqlite3_exec(db, sql, NULL, NULL, NULL);
                    RefreshListView(libraryDataGrid, db, false);
                }
            }
            break;
        }
        break;
        case 5: // Test Email Button
        {
            // Notify the user so they know something is happening
            MessageBox(hWnd, L"Launching Python Email Script...", L"Email Test", MB_OK | MB_ICONINFORMATION);

            // Launch the script
            // Note: Ensure weekly_reminder.py is in the same folder as your .exe
            HINSTANCE result = ShellExecuteA(NULL, "open", "python.exe", "email_script.py", NULL, SW_SHOW);

            // Check if the script actually started
            if ((INT_PTR)result <= 32) {
                MessageBox(hWnd, L"Failed to launch Python script. Check if python.exe is in your PATH.", L"Error", MB_ICONERROR);
            }
        }
        break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        } // End of switch(wmId)
    } // End of case WM_COMMAND
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_CREATE:
    {
        HWND addButton = CreateWindowEx(0, L"BUTTON", L"Add",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            1700, 10, 150, 30,
            hWnd, (HMENU)2, hInst, nullptr);

        /*HWND deleteButton = CreateWindowEx(0, L"BUTTON", L"Delete",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            1700, 50, 150, 30,
        hWnd, (HMENU)4, hInst, nullptr);*/

        // NEW: Test Email Button (ID: 5)
        HWND emailButton = CreateWindowEx(0, L"BUTTON", L"Manual Send Email",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            1700, 50, 150, 30, // Positioned below the Add button
            hWnd, (HMENU)5, hInst, nullptr);
    }

    break;
    case WM_CLOSE:
        CheckAndSendWeeklyEmails();
        sqlite3_close(db);

        sqlite3_close(db_test);
        DestroyWindow(hWnd);
        break;
    
    case WM_DESTROY:
       sqlite3_close(db);
     
		sqlite3_close(db_test);
        PostQuitMessage(0);
        break;
    case WM_NOTIFY:
    {
        LPNMHDR lpnmh = (LPNMHDR)lParam;
        // Check if the notification is from your DataGrid and is a Right Click
        if (lpnmh->hwndFrom == libraryDataGrid && lpnmh->code == NM_RCLICK) {
            LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lParam;

            // Ensure the user actually clicked on an item, not empty space
            if (lpnmlv->iItem != -1) {
                HMENU hPopupMenu = CreatePopupMenu();
                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 1001, L"Mark as Returned");
                InsertMenu(hPopupMenu, 1, MF_BYPOSITION | MF_STRING, 1002, L"Mark as Not Returned");
                InsertMenu(hPopupMenu, 2, MF_SEPARATOR, 0, NULL); // Visual separator
                InsertMenu(hPopupMenu, 3, MF_BYPOSITION | MF_STRING, 1003, L"Delete Record");

                POINT pt;
                GetCursorPos(&pt);
                TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
                DestroyMenu(hPopupMenu);
            }
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK MyDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {// Use this to set default values when the dialog first opens
        HWND hEdit = GetDlgItem(hDlg, IDC_MY_EDITBOX);
        HWND hBook = GetDlgItem(hDlg, IDC_EDIT1);
        Edit_SetCueBannerText(hEdit, L"Enter ID...");
        Edit_SetCueBannerText(hBook, L"Enter Book ID");
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        // Handle button clicks or text changes
        switch (LOWORD(wParam)) {
        case IDOK: {
            // Get text from an input box before closing
            wchar_t szStudent[256], szBook[256];
            char studentID[20], bookID[20];

            GetDlgItemText(hDlg, IDC_MY_EDITBOX, szStudent, 256);
            GetDlgItemText(hDlg, IDC_EDIT1, szBook, 256);

            RemoveAllSpacesW(szStudent);
            RemoveAllSpacesW(szBook);

            // 2. CHECK: Is the input empty?
            if (wcslen(szStudent) == 0 || wcslen(szBook) == 0) {
                MessageBox(hDlg, L"Please enter both a Student ID and a Book ID!", L"Incomplete Data", MB_ICONWARNING);
                return (INT_PTR)TRUE;
            }


            sprintf_s(studentID, "%ls", szStudent);
            sprintf_s(bookID, "%ls", szBook);

            // 3. Try to find and copy the data
            sqlite3_exec(db_test, "ATTACH DATABASE 'test_data.db' AS test_source;", NULL, NULL, NULL);

            char query[2048];
            sprintf_s(query,
                "INSERT INTO librarylog (StudentID, StudentName, Year_Section, Email, Status, BookBorrowed) "
                "SELECT s.StudentID, s.Name, s.Section, s.Email, s.Status, b.BookName "
                "FROM test_source.students s, test_source.books b "
                "WHERE REPLACE(s.StudentID, ' ', '') = '%s' "
                "AND REPLACE(b.BookID, ' ', '') = '%s' LIMIT 1;",
                studentID, bookID);

            int rc = sqlite3_exec(db, query, NULL, NULL, NULL);
            int rowsChanged = sqlite3_changes(db);

            sqlite3_exec(db, "DETACH DATABASE test_source;", NULL, NULL, NULL);

            // 4. VALIDATION LOGIC
            if (rc == SQLITE_OK && rowsChanged > 0) {
                // SUCCESS: Data found and copied
                MessageBox(hDlg, L"Student Added Successfully!", L"Success", MB_OK);
                RefreshListView(libraryDataGrid, db, true);
                // Refresh your main window's List View here
                // UpdateMainListView(); 

                EndDialog(hDlg, IDOK); // ONLY close the dialog if data was valid and processed
            }
            else {
                // FAIL: ID not in test DB
                MessageBox(hDlg, L"Student ID or Book ID not found in database. Please try again.", L"Error", MB_ICONERROR);

                // Clear the text box so they can try again
                SetDlgItemText(hDlg, IDC_MY_EDITBOX, L"");
                SetDlgItemTextW(hDlg, IDC_EDIT1, L"");
                SetFocus(GetDlgItem(hDlg, IDC_MY_EDITBOX));

                return TRUE; // Keep the dialog box OPEN
            }
        }
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;


}

INT_PTR CALLBACK DeleteDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
    {// Use this to set default values when the dialog first opens
        HWND hEdit = GetDlgItem(hDlg, DialogDeleteBox);
        Edit_SetCueBannerText(hEdit, L"Enter row to delete");
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        // Handle button clicks or text changes
        switch (LOWORD(wParam)) {
        case DialogDelete:
        {
            // Get text from an input box before closing
            wchar_t szStudent[256];
            char studentID[20];
            GetDlgItemText(hDlg, DialogDeleteBox, szStudent, 256);
        }
        case DialogDeleteCancel:
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
        break;

    }
    return (INT_PTR)FALSE;
}

