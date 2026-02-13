// library app.cpp : Defines the entry point for the application.
//

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

	    libraryDataGridColumns.pszText = (LPWSTR) L"No.";
	    ListView_InsertColumn(libraryDataGrid, 0, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Student ID";
        ListView_InsertColumn(libraryDataGrid, 1, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Name";
        ListView_InsertColumn(libraryDataGrid, 2, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Year & Section";
        ListView_InsertColumn(libraryDataGrid, 3, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Book Borrowed";
	    ListView_InsertColumn(libraryDataGrid, 4, &libraryDataGridColumns);
        libraryDataGridColumns.pszText = (LPWSTR) L"Return Date";
	    ListView_InsertColumn(libraryDataGrid, 5, &libraryDataGridColumns);
		libraryDataGridColumns.pszText = (LPWSTR) L"Status";
        ListView_InsertColumn(libraryDataGrid, 6, &libraryDataGridColumns);


		// Insert sample data
        
        LVITEM lvi = {0};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = ListView_GetItemCount(libraryDataGrid ); // Put it at the bottom
        lvi.iSubItem = 0;


        // Insert the row
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
void ShowCellComboBox(int row, int col) {
    RECT rect;
    // LVIR_BOUNDS is standard; for Column 0 on Win7, use LVIR_LABEL
    ListView_GetSubItemRect(libraryDataGrid, row, col, LVIR_BOUNDS, &rect);

    MoveWindow(comboBox, rect.left, rect.top,
        rect.right - rect.left, 100, // Height is for the dropdown area
        TRUE);

    ShowWindow(comboBox, SW_SHOW);
    SetFocus(comboBox);
}

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
            int gridWidth = (int)(winWidth * 0.75) - padding;

            // MATH: Keep your dynamic height calculation
            int gridHeight = winHeight - topSectionHeight - padding;

            // Apply the size
            // We keep it at X=padding so the 20% gap stays on the RIGHT
            SetWindowPos(hGrid, NULL, padding, topSectionHeight, gridWidth, gridHeight, SWP_NOZORDER);

            ListView_SetColumnWidth(hGrid, 0, 50);
            ListView_SetColumnWidth(hGrid, 1, 150);
            ListView_SetColumnWidth(hGrid, 2, 400);
            ListView_SetColumnWidth(hGrid, 3, 150);
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
                1500, 10, 150, 30,
				hWnd, (HMENU)2, hInst, nullptr);
        }
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            // ADD THIS LINE to define lpnmia
            LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;

            if (pnmh->idFrom == 1) // Your ListView ID
            {
                if (pnmh->code == NM_CLICK || pnmh->code == NM_DBLCLK)
                {
                    int targetColumn = 6;

                    // Now 'lpnmia' is recognized by the compiler
                    if (lpnmia->iSubItem == targetColumn && lpnmia->iItem != -1) {
                        ShowCellComboBox(lpnmia->iItem, lpnmia->iSubItem);
                    }
                    else {
                        ShowWindow(comboBox, SW_HIDE);
                    }
                }
            }
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

            // Close the dialog and return a value (e.g., 1 for success)
            EndDialog(hDlg, 1);
            return (INT_PTR)TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
