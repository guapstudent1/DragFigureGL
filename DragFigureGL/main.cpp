#include <windows.h>		// Header File For Windows
#include <tchar.h>
#include <stdio.h>			// Header File For Standard Input/Output
#include <math.h>			// Header File For The Math Library
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

#include "engine.h"



HDC         hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;	// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default


GLfloat xpos = 3.9f;
GLfloat ypos = 3.4f;
GLfloat hold;					// Temporarily Holds A Floating Point Value
GLfloat zoom = 1.0f;
GLfloat xyzoom = 1;
GLuint  memlist;				// Memory for object


#define N	3					// Count of scene objects

SCENE_ELEMENT sc[N];			// Scene array of objects
int neari = -1;					// Index of scene active object 

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
    if (height == 0)									// Prevent A Divide By Zero By
    {
        height = 1;										// Making Height Equal One
    }

    glViewport(0, 0, width, height);					// Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
    glLoadIdentity();									// Reset The Projection Matrix

    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
    glLoadIdentity();									// Reset The Modelview Matrix
}
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
    /*if (!LoadGLTextures())							// Jump To Texture Loading Routine ( NEW )
    {
        return FALSE;									// If Texture Didn't Load Return FALSE
    }*/

    glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
    glClearColor(1.0f, 1.0f, 1.0f, 0.5f);				// Black Background
    glClearDepth(1.0f);									// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
    glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
    glPolygonMode(GL_BACK, GL_FILL);					// Back Face Is Solid
    glPolygonMode(GL_FRONT, GL_LINE);					// Front Face Is Made Of Lines

    BuildLists();
    InitFigurePos(sc, N);
    return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
    glLoadIdentity();

    glTranslatef(xpos, ypos, -12.0f);					// Reset The View
    glScalef(zoom, zoom, 1.0f);
    if (zoom < 0.001f)zoom = 0.001f;

    // Here we draw all objects
    for (int i = 0; i < N; i++) {
        float theta = 2.0f * 3.1415926f * i / N;
        if (i == neari)
        {
            glColor3f(0.4f, 0.1f, 0.4f);
        }
        else
        {
            glColor3f(0.1f, 0.1f, 0.4f);
        }
        glTranslatef(sc[i].xpos, sc[i].ypos, 0.0f);
        glCallList(memlist);
        glTranslatef(-sc[i].xpos, -sc[i].ypos, 0.0f);

    }

    return TRUE;										// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
    if (fullscreen)										// Are We In Fullscreen Mode?
    {
        ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
        ShowCursor(TRUE);								// Show Mouse Pointer
    }

    if (hRC)											// Do We Have A Rendering Context?
    {
        if (!wglMakeCurrent(NULL, NULL))				// Are We Able To Release The DC And RC Contexts?
        {
            MessageBox(NULL, _T("Release Of DC And RC Failed."), _T("SHUTDOWN ERROR"), MB_OK | MB_ICONINFORMATION);
        }

        if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
        {
            MessageBox(NULL, _T("Release Rendering Context Failed."), _T("SHUTDOWN ERROR"), MB_OK | MB_ICONINFORMATION);
        }
        hRC = NULL;										// Set RC To NULL
    }

    if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
        MessageBox(NULL, _T("Release Device Context Failed."), _T("SHUTDOWN ERROR"), MB_OK | MB_ICONINFORMATION);
        hDC = NULL;										// Set DC To NULL
    {
    }

    if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
    {
        MessageBox(NULL, _T("Could Not Release hWnd."), _T("SHUTDOWN ERROR"), MB_OK | MB_ICONINFORMATION);
        hWnd = NULL;										// Set hWnd To NULL
    }

    if (!UnregisterClass(_T("OpenGL"), hInstance))			// Are We Able To Unregister Class
    {
        MessageBox(NULL, _T("Could Not Unregister Class."), _T("SHUTDOWN ERROR"), MB_OK | MB_ICONINFORMATION);
        hInstance = NULL;									// Set hInstance To NULL
    }
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(TCHAR* title, int width, int height, BYTE bits, bool fullscreenflag)
{
    GLuint		PixelFormat;			// Holds The Results After Searching For A Match
    WNDCLASS	wc;						// Windows Class Structure
    DWORD		dwExStyle;				// Window Extended Style
    DWORD		dwStyle;				// Window Style
    RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
    WindowRect.left = (long)0;			// Set Left Value To 0
    WindowRect.right = (long)width;		// Set Right Value To Requested Width
    WindowRect.top = (long)0;				// Set Top Value To 0
    WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

    fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

    hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
    wc.lpfnWndProc = (WNDPROC)WndProc;				// WndProc Handles Messages
    wc.cbClsExtra = 0;								// No Extra Window Data
    wc.cbWndExtra = 0;								// No Extra Window Data
    wc.hInstance = hInstance;						// Set The Instance
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
    wc.hCursor = LoadCursor(NULL, IDC_HAND);		// Load The Arrow Pointer
    wc.hbrBackground = NULL;						// No Background Required For GL
    wc.lpszMenuName = NULL;							// We Don't Want A Menu
    wc.lpszClassName = _T("OpenGL");				// Set The Class Name

    if (!RegisterClass(&wc))						// Attempt To Register The Window Class
    {
        MessageBox(NULL, _T("Failed To Register The Window Class."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;											// Return FALSE
    }

    if (fullscreen)												// Attempt Fullscreen Mode?
    {
        DEVMODE dmScreenSettings;								// Device Mode
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth = width;				    // Selected Screen Width
        dmScreenSettings.dmPelsHeight = height;				    // Selected Screen Height
        dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
            if (MessageBox(NULL, _T("The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?"), _T("NeHe GL"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
            {
                fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
            }
            else
            {
                // Pop Up A Message Box Letting User Know The Program Is Closing.
                MessageBox(NULL, _T("Program Will Now Close."), _T("ERROR"), MB_OK | MB_ICONSTOP);
                return FALSE;									// Return FALSE
            }
        }
    }

    if (fullscreen)												// Are We Still In Fullscreen Mode?
    {
        dwExStyle = WS_EX_APPWINDOW;							// Window Extended Style
        dwStyle = WS_POPUP;										// Windows Style
        //ShowCursor(FALSE);									// Hide Mouse Pointer
    }
    else
    {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
        dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
    }

    AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

    // Create The Window
    if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
        _T("OpenGL"),						// Class Name
        title,								// Window Title
        dwStyle |							// Defined Window Style
        WS_CLIPSIBLINGS |					// Required Window Style
        WS_CLIPCHILDREN,					// Required Window Style
        0, 0,								// Window Position
        WindowRect.right - WindowRect.left,	// Calculate Window Width
        WindowRect.bottom - WindowRect.top,	// Calculate Window Height
        NULL,								// No Parent Window
        NULL,								// No Menu
        hInstance,							// Instance
        NULL)))								// Dont Pass Anything To WM_CREATE
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Window Creation Error."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
        1,											// Version Number
        PFD_DRAW_TO_WINDOW |						// Format Must Support Window
        PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
        PFD_DOUBLEBUFFER,							// Must Support Double Buffering
        PFD_TYPE_RGBA,								// Request An RGBA Format
        bits,										// Select Our Color Depth
        0, 0, 0, 0, 0, 0,							// Color Bits Ignored
        0,											// No Alpha Buffer
        0,											// Shift Bit Ignored
        0,											// No Accumulation Buffer
        0, 0, 0, 0,									// Accumulation Bits Ignored
        16,											// 16Bit Z-Buffer (Depth Buffer)  
        0,											// No Stencil Buffer
        0,											// No Auxiliary Buffer
        PFD_MAIN_PLANE,								// Main Drawing Layer
        0,											// Reserved
        0, 0, 0										// Layer Masks Ignored
    };

    if (!(hDC = GetDC(hWnd)))						// Did We Get A Device Context?
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Can't Create A GL Device Context."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Can't Find A Suitable PixelFormat."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    if (!SetPixelFormat(hDC, PixelFormat, &pfd))	// Are We Able To Set The Pixel Format?
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Can't Set The PixelFormat."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Can't Create A GL Rendering Context."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Can't Activate The GL Rendering Context."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    ShowWindow(hWnd, SW_SHOW);						// Show The Window
    SetForegroundWindow(hWnd);						// Slightly Higher Priority
    SetFocus(hWnd);									// Sets Keyboard Focus To The Window
    ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

    if (!InitGL())									// Initialize Our Newly Created GL Window
    {
        KillGLWindow();								// Reset The Display
        MessageBox(NULL, _T("Initialization Failed."), _T("ERROR"), MB_OK | MB_ICONEXCLAMATION);
        return FALSE;								// Return FALSE
    }

    return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
    UINT	uMsg,								// Message For This Window
    WPARAM	wParam,								// Additional Message Information
    LPARAM	lParam)								// Additional Message Information
{
    static GLfloat dx = 0;
    static GLfloat dy = 0;
    static GLboolean initXY = false;
    switch (uMsg)								// Check For Windows Messages
    {
    case WM_ACTIVATE:							// Watch For Window Activate Message
    {
        if (!HIWORD(wParam))					// Check Minimization State
        {
            active = TRUE;						// Program Is Active
        }
        else
        {
            active = FALSE;						// Program Is No Longer Active
        }

        return 0;								// Return To The Message Loop
    }

    case WM_SYSCOMMAND:							// Intercept System Commands
    {
        switch (wParam)							// Check System Calls
        {
        case SC_SCREENSAVE:						// Screensaver Trying To Start?
        case SC_MONITORPOWER:					// Monitor Trying To Enter Powersave?
            return 0;							// Prevent From Happening
        }
        break;									// Exit
    }

    case WM_CLOSE:								// Did We Receive A Close Message?
    {
        PostQuitMessage(0);						// Send A Quit Message
        return 0;								// Jump Back
    }

    case WM_KEYDOWN:							// Is A Key Being Held Down?
    {
        keys[wParam] = TRUE;					// If So, Mark It As TRUE
        return 0;								// Jump Back
    }

    case WM_KEYUP:								// Has A Key Been Released?
    {
        keys[wParam] = FALSE;					// If So, Mark It As FALSE
        return 0;								// Jump Back
    }

    case WM_SIZE:										// Resize The OpenGL Window
    {
        ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
        return 0;										// Jump Back
    }
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    {
        if ((wParam & MK_LBUTTON) || uMsg == WM_LBUTTONUP)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            // Translate coordinates
            POINT pt = { x,y };
            ClientToScreen(hWnd, &pt);
            // Detect active object
            GLint viewport[4];
            GLdouble modelview[16];
            GLdouble projection[16];
            GLdouble PosX, PosY, x1, x2, x3;

            glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
            glGetDoublev(GL_PROJECTION_MATRIX, projection);

            glGetIntegerv(GL_VIEWPORT, viewport);

            PosX = (float)pt.x;
            PosY = (float)viewport[3] - (float)pt.y;

            gluUnProject(PosX, PosY, 0.9927f, modelview, projection, viewport, &x1, &x2, &x3);

            float xpos = (float)x1;
            float ypos = (float)x2;


            float minx = 1.4f;
            float miny = 1.4f;
            int neartest = 0;
            for (int i = 0; i < N; i++)
            {
                if (fabsf(sc[i].xpos - xpos) < minx && fabsf(sc[i].ypos - ypos) < miny) {
                    minx = fabsf(sc[i].xpos - xpos);
                    miny = fabsf(sc[i].ypos - ypos);
                    neartest = i;
                }
            }
            if (minx * minx + miny * miny > 1.4 * 1.4)
            {
                neari = -1;
            }
            else
            {
                neari = neartest;

                dx = sc[neari].xpos - xpos;
                dy = sc[neari].ypos - ypos;
                initXY = true;
            }
            if (uMsg == WM_LBUTTONUP) {
                initXY = false;
            }
            return 0;
        }
    }
    case WM_MOUSEMOVE:
    {

        if (wParam & MK_LBUTTON && initXY)
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            // Translate coordinates
            POINT pt = { x, y };
            ClientToScreen(hWnd, &pt);


            GLint viewport[4];
            GLdouble modelview[16];
            GLdouble projection[16];
            GLdouble PosX, PosY, x1, x2, x3;

            glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
            glGetDoublev(GL_PROJECTION_MATRIX, projection);

            glGetIntegerv(GL_VIEWPORT, viewport);

            PosX = (float)pt.x;
            PosY = (float)viewport[3] - (float)pt.y;

            gluUnProject(PosX, PosY, 0.9927f, modelview, projection, viewport, &x1, &x2, &x3);

            float xpos = (float)x1;
            float ypos = (float)x2;

            // Move position of active object
            if (neari >= 0 && neari < N)
            {
                sc[neari].xpos = xpos + dx;
                sc[neari].ypos = ypos + dy;
            }

            return 0;
        }
    }
    }

    // Pass All Unhandled Messages To DefWindowProc
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
    HINSTANCE	hPrevInstance,		                // Previous Instance
    LPSTR		lpCmdLine,			                // Command Line Parameters
    int			nCmdShow)			                // Window Show State
{
    MSG		msg;									// Windows Message Structure
    BOOL	done = FALSE;								// Bool Variable To Exit Loop

    // Ask The User Which Screen Mode They Prefer
    if (MessageBox(NULL, _T("Would You Like To Run In Fullscreen Mode?"), _T("Start FullScreen?"), MB_YESNO | MB_ICONQUESTION) == IDNO)
    {
        fullscreen = FALSE;							// Windowed Mode
    }

    // Create Our OpenGL Window
    if (!CreateGLWindow(_T("Linfuong & NeHe's Object Drag'n'Drop Tutorial"), 1366, 768, 32, fullscreen))
    {
        return 0;									// Quit If Window Was Not Created
    }

    while (!done)									// Loop That Runs While done=FALSE
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
        {
            if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
            {
                done = TRUE;							// If So done=TRUE
            }
            else									// If Not, Deal With Window Messages
            {
                TranslateMessage(&msg);				// Translate The Message
                DispatchMessage(&msg);				// Dispatch The Message
            }
        }
        else										// If There Are No Messages
        {
            // Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
            if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
            {
                done = TRUE;							// ESC or DrawGLScene Signalled A Quit
            }
            else									// Not Time To Quit, Update Screen
            {
                SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
                if (keys[VK_UP])
                {
                    ypos -= 0.004f * xyzoom;

                }

                if (keys[VK_DOWN])
                {
                    ypos += 0.004f * xyzoom;
                }
                if (keys[VK_RIGHT])
                {
                    xpos -= 0.004f * xyzoom;
                }

                if (keys[VK_LEFT])
                {

                    xpos += 0.004f * xyzoom;
                }
                if (keys[VK_PRIOR])             // Page Up pushed?
                {
                    zoom /= 1.002f;              // Decrease
                    xyzoom /= 1.002f;
                }

                if (keys[VK_NEXT])              // Page Down pushed?
                {
                    zoom *= 1.002f;             // Increase
                    xyzoom *= 1.002f;
                }
            }

            if (keys[VK_F1])						// Is F1 Being Pressed?
            {
                keys[VK_F1] = FALSE;					// If So Make Key FALSE
                KillGLWindow();						// Kill Our Current Window
                fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
                // Recreate Our OpenGL Window
                if (!CreateGLWindow(_T("LinFuong & NeHe's Object Drag'n'Drop Tutorial"), 1366, 768, 32, fullscreen))
                {
                    return 0;						// Quit If Window Was Not Created
                }
            }
        }
    }

    // Shutdown
    KillGLWindow();									// Kill The Window
    return (msg.wParam);							// Exit The Program
}
