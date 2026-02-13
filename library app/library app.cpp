// library app.cpp : Defines the entry point for the application.
//

#include <windowsx.h>
#include <cstdio>
#include "framework.h"
#include "library app.h"
#include <commctrl.h>
#include "sqlite3.h"
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

sqlite3* db = nullptr;
sqlite3* db_test = nullptr;

// Helper function to execute SQL
void ExecuteSQL(const char* sql) {
    char* errorMessage = nullptr;
    int rc = sqlite3_exec(db, sql, NULL, 0, &errorMessage);
    if (rc != SQLITE_OK) {
        MessageBoxA(NULL, errorMessage, "SQLite Error", MB_ICONERROR);
        sqlite3_free(errorMessage);
    }
}

void RefreshListView(HWND hWndListView, sqlite3* db_local) {
    // 1. Clear existing items
    ListView_DeleteAllItems(hWndListView);

    // 2. Prepare the SELECT query
    const char* query = "SELECT StudentID, StudentName, Year_Section, Email, BookBorrowed, ReturnDate , Status FROM librarylog;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_local, query, -1, &stmt, NULL) == SQLITE_OK) {
        int index = 0;

        // 3. Loop through each row in the database
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* id = sqlite3_column_text(stmt, 0);
            const unsigned char* name = sqlite3_column_text(stmt, 1);
            const unsigned char* section = sqlite3_column_text(stmt, 2);
            const unsigned char* email = sqlite3_column_text(stmt, 3);
            const unsigned char* book = sqlite3_column_text(stmt, 4);
            int date = sqlite3_column_int(stmt, 5);
			const unsigned char* status = sqlite3_column_text(stmt, 6);

            // 4. Insert into ListView
            LVITEM lvItem = { 0 };
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = index;

            // Column 0: Student ID (Convert UTF-8 to Wide String)
            wchar_t wText[256];
            swprintf_s(wText, L"%hs", id);
            lvItem.pszText = wText;
            ListView_InsertItem(hWndListView, &lvItem);

            // Column 1: Name
            swprintf_s(wText, L"%hs", name);
            ListView_SetItemText(hWndListView, index, 1, wText);

            // Column 2: Section
            swprintf_s(wText, L"%hs", section);
            ListView_SetItemText(hWndListView, index, 2, wText);

            swprintf_s(wText, L"%hs", email);
            ListView_SetItemText(hWndListView, index, 3, wText);

            swprintf_s(wText, L"%hs", status);
            ListView_SetItemText(hWndListView, index, 6, wText);

            // ... repeat SetItemText for Book and Date ...

            index++;
        }
        sqlite3_finalize(stmt);
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
        "CREATE TABLE IF NOT EXISTS students (StudentID INTEGER PRIMARY KEY, Name TEXT, Section TEXT, Email TEXT, Status TEXT);"
        "INSERT OR IGNORE INTO students VALUES (123456, 'Vince michael', 'BSIT-1A', '123456@gende.com', 'not returned');"
        "INSERT OR IGNORE INTO students VALUES (789012, 'Maria Clara', 'BSCS-2B', '789012@gende.com', 'not returned');";

    sqlite3_exec(db_test, setup_test, NULL, NULL, NULL);
    sqlite3_close(db_test); // Close it so it's ready to be ATTACHED later

    

    // Open or create the local database file
    if (sqlite3_open("local_data.db", &db) != SQLITE_OK) {
        MessageBox(NULL, L"Failed to open database! main", L"Error", MB_OK);
        return 0;
    }

    const char* schema =
        "CREATE TABLE IF NOT EXISTS librarylog ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "StudentID INTEGER DEFAULT 000000, "
        "StudentName TEXT, "
        "Year_Section TEXT,"
		"Email TEXT,"
        "BookBorrowed TEXT,"
        "ReturnDate INTEGER,"
		"Status TEXT DEFAULT 'Not Returned'"
        ");";
        sqlite3_exec(db, schema, NULL, NULL, NULL);
    

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

   /*
   HWND hInputEdit = CreateWindowEx(
       0,
       L"EDIT", // Predefined class for input
       NULL,
       WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
       10, 10, 200, 25, // x, y, width, height
       hWnd, // Parent window handle
       (HMENU)6969, // Unique ID
       hInst,
       NULL
   );
   */

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

        libraryDataGridColumns.pszText = (LPWSTR) L"Student ID";
        ListView_InsertColumn(libraryDataGrid, 0, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Name";
        ListView_InsertColumn(libraryDataGrid, 1, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Year & Section";
        ListView_InsertColumn(libraryDataGrid, 2, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR)L"Email Adress";
        ListView_InsertColumn(libraryDataGrid, 3, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Book Borrowed";
	    ListView_InsertColumn(libraryDataGrid, 4, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Return Date";
	    ListView_InsertColumn(libraryDataGrid, 6, &libraryDataGridColumns);
		libraryDataGridColumns.pszText = (LPWSTR) L"Status";
        ListView_InsertColumn(libraryDataGrid, 7, &libraryDataGridColumns);


		// Insert sample data
        RefreshListView(libraryDataGrid, db);


       /*
        int index = ListView_InsertItem(libraryDataGrid, &lvi);
        
        ListView_SetItemText(libraryDataGrid, index, 0, (LPWSTR)L"1");
        ListView_SetItemText(libraryDataGrid, index, 1, (LPWSTR)L"451859");
        ListView_SetItemText(libraryDataGrid, index, 2, (LPWSTR)L"ron");
        ListView_SetItemText(libraryDataGrid, index, 3, (LPWSTR)L"12 - Galileo");
        ListView_SetItemText(libraryDataGrid, index, 4, (LPWSTR)L"kwento ng isang bisaya - shynn");
        ListView_SetItemText(libraryDataGrid, index, 5, (LPWSTR)L"next friday");

        ListView_InsertItem(libraryDataGrid, &lvi);

        ListView_SetItemText(libraryDataGrid, index, 0, (LPWSTR)L"2");
        ListView_SetItemText(libraryDataGrid, index, 1, (LPWSTR)L"451859");
        ListView_SetItemText(libraryDataGrid, index, 2, (LPWSTR)L"ron");
        ListView_SetItemText(libraryDataGrid, index, 3, (LPWSTR)L"12 - test");
        ListView_SetItemText(libraryDataGrid, index, 4, (LPWSTR)L"kwento ng isang bisaya - shynn");
        ListView_SetItemText(libraryDataGrid, index, 5, (LPWSTR)L"next friday");
        */
    
        
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

            ListView_SetColumnWidth(hGrid, 0, 150);
            ListView_SetColumnWidth(hGrid, 1, 400);
            ListView_SetColumnWidth(hGrid, 2, 150);
			ListView_SetColumnWidth(hGrid, 3, 300);
            ListView_SetColumnWidth(hGrid, 4, 300);
            ListView_SetColumnWidth(hGrid, 5, 200);
            ListView_SetColumnWidth(hGrid, 6, LVSCW_AUTOSIZE_USEHEADER);
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
            // Parse the menu selections:

            switch (wmId)
            {
            case 2:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, MyDialogProc);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
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
        }

		break;
    case WM_DESTROY:
       // sqlite3_close(db);
     {// 1. Wipe the data from your local log
        const char* cleanup = "DELETE FROM librarylog;";
        sqlite3_exec(db, cleanup, NULL, NULL, NULL);

        // 2. Optional: Reset the ID counter to 1
        sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='librarylog';", NULL, NULL, NULL);

        // 3. Close the database handle
        sqlite3_close(db);
    }
    
		sqlite3_close(db_test);
        PostQuitMessage(0);
        break;
    case WM_NOTIFY:
    {
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
        Edit_SetCueBannerText(hEdit, L"Enter ID...");
        return (INT_PTR)TRUE;
    }
    case WM_COMMAND:
        // Handle button clicks or text changes
        switch (LOWORD(wParam)) {
        case IDOK: {
            // Get text from an input box before closing
            wchar_t buffer[256];
            GetDlgItemText(hDlg, IDC_MY_EDITBOX, buffer, 256);

            // 2. CHECK: Is the input empty?
            if (wcslen(buffer) == 0) {
                MessageBox(hDlg, L"Please enter a Student ID!", L"Incomplete Data", MB_ICONWARNING);
                return TRUE; // Returning TRUE here prevents the dialog from closing
            }

            char studentID[20];
            sprintf_s(studentID, "%ls", buffer);

            // 3. Try to find and copy the data
            sqlite3_exec(db_test, "ATTACH DATABASE 'test_data.db' AS test_source;", NULL, NULL, NULL);

            char query[2048];
            sprintf_s(query,
                "INSERT INTO librarylog (StudentID, StudentName, Year_Section, Email, Status) "
                "SELECT StudentID, Name, Section, Email, Status FROM test_source.students "
                "WHERE StudentID = '%s';", studentID);

            int rc = sqlite3_exec(db_test, query, NULL, NULL, NULL);
            int rowsChanged = sqlite3_changes(db);

            sqlite3_exec(db, "DETACH DATABASE test_source;", NULL, NULL, NULL);

            // 4. VALIDATION LOGIC
            if (rc == SQLITE_OK && rowsChanged > 0) {
                // SUCCESS: Data found and copied
                MessageBox(hDlg, L"Student Added Successfully!", L"Success", MB_OK);
                RefreshListView(libraryDataGrid, db);
                // Refresh your main window's List View here
                // UpdateMainListView(); 

                EndDialog(hDlg, IDOK); // ONLY close the dialog if data was valid and processed
            }
            else {
                // FAIL: ID not in test DB
                MessageBox(hDlg, L"Student ID not found in database. Please try again.", L"Error", MB_ICONERROR);

                // Clear the text box so they can try again
                SetDlgItemText(hDlg, IDC_MY_EDITBOX, L"");
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

