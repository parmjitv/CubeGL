/*
 * main.c: Renders a spinning cube via OpenGL (Windows)
 *
 * Authored by Parmjit Virk (2017)
 * 
 * Licensed under the MIT license as per the Open Source Initiative 2017.
 * See the LICENSE file for the complete license information,
 * or visit https://opensource.org/licenses/MIT for details.
 *
 */

#include <windows.h>
#include <gl/gl.h>

/* Pixel dimensions of rendering window */
#define HEIGHT 300
#define WIDTH 300

/* Declare function prototypes */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM); /* windows proc */
void InitGLState(void);
void RenderCube(void);
void CreateCubeWire(void);
void CreateCubeSolid(void);

/* Handle to window drawing context */
HDC hdc;

int WINAPI WinMain (
                     HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nFunsterStil
                   )
  {
    char szClassName[] = "CubeGL";

    HWND hwnd;         /* This is the handle for our window */
    MSG messages;      /* Here messages to the application are saved */
    WNDCLASSEX wincl;  /* Data structure for the windowclass */

    long pf;
    PIXELFORMATDESCRIPTOR pfd; /* GL pixel format structure */
    HGLRC hrc;

    /* The Window structure */
    wincl.hInstance     = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc   = WindowProcedure;  /* This function is called by windows */
    wincl.style         = CS_DBLCLKS;       /* Catch double-clicks */
    wincl.cbSize        = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon        = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm      = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;  /* No menu */
    wincl.cbClsExtra   = 0;     /* No extra bytes after the window class */
    wincl.cbWndExtra   = 0;     /* structure or the window instance */

    /* Register the window class, and if it fails quit the program */
    if (! RegisterClassEx(&wincl))
      {
        return(0);
      }

    /* The class is registered, let's create the window */
    hwnd = CreateWindowEx
            (
              0,                   /* Extended possibilites for variation */
              szClassName,         /* Classname */
              "CubeGL",           /* Title Text */
              WS_OVERLAPPEDWINDOW, /* default window */
              /*WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,*/
              CW_USEDEFAULT,       /* Windows decides the position */
              CW_USEDEFAULT,       /* where the window ends up on the screen */
              WIDTH,               /* The programs width */
              HEIGHT,              /* and height in pixels */
              NULL,                /* The window is not a child-window to desktop */
              NULL,                /* No menu */
              hThisInstance,       /* Program Instance handler */
              NULL                 /* No Window Creation data */
            );

    /* If no handle could be created, quit the program */
    if (hwnd != NULL)
      {
        hdc = GetDC(hwnd);

        ZeroMemory(&pfd, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;

        pf = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pf, &pfd);

        hrc = wglCreateContext(hdc);
        wglMakeCurrent(hdc, hrc);

        /* Make the window visible on the screen */
        ShowWindow(hwnd, nFunsterStil);

        /* initialize the GL state for our window */
        InitGLState();

        /* send the window a WM_PAINT message */
        UpdateWindow(hwnd);
      }
    else
      {
        return(0);
      }

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage(&messages, NULL, 0, 0))
      {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);

        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
      }

    /* cleanup code */
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return(messages.wParam);
  }

/* This function is called by the Windows function DispatchMessage() */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    static PAINTSTRUCT ps;
    WPARAM key;

    /* handle the messages */
    switch (message)
      {
        case WM_CREATE:
          /* setup a windows timer for animation */
          SetTimer(hwnd, 101, 10, NULL);
          break;

        case WM_TIMER:
          /* force a redraw of the entire window contents */
          InvalidateRect(hwnd, NULL, FALSE);
          break;

        case WM_PAINT:
          /* render our spinning cube */
          RenderCube();
          BeginPaint(hwnd, &ps);
          EndPaint(hwnd, &ps);
          break;

        case WM_KEYDOWN:
          key = (int)wParam;

          if (key == 0x31) /* a key is pressed, so destroy the window */
            {
              break;
            }

        case WM_DESTROY:
          /* send a WM_QUIT to the message queue */
          KillTimer(hwnd, 101);
          PostQuitMessage(0);
          break;

        default:
          /* for messages that we don't deal with */
          return(DefWindowProc(hwnd, message, wParam, lParam));
      }

    return(0);
  }

void InitGLState(void)
  {
    GLfloat nRange = 30.0f; /* units of world coordinates */
    
    /* GL_SMOOTH - shade solid models with smooth colors */
    glShadeModel(GL_SMOOTH);
    /* GL_FLAT - no shading */
    /*glShadeModel(GL_FLAT);*/

    /* enable depth buffer and set background color */
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    /* clamp world cooordinates to window coordinates */
    glViewport(0, 0, WIDTH, HEIGHT);
    glOrtho(-nRange, nRange, -nRange, nRange, -nRange, nRange);

    /* push identity matrix onto the stack */
    glPushMatrix(); 
  }

void RenderCube(void)
  {
    /* degrees per frame, use higher values for faster rotation */
    int rotateAngle = 5;

    static int frameCount = 0;

    static float xRot = 1.0,
                 yRot = 1.0,
                 zRot = 1.0;

    frameCount++;
    if (frameCount == 25)
      {
        xRot = (float)rand();
      }
    else if (frameCount == 30)
      {
        yRot = (float)rand();
      }
    else if (frameCount == 35)
      {
        zRot = (float)rand();
        frameCount = 0;
      }

    /* clear color and depth buffers for each frame drawn */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* pop the identity matrix from the stack and rotate the viewing frame */
    glPopMatrix(); 
    glRotatef(rotateAngle, xRot, yRot, zRot);

    /* render a wireframe cube */
    /*CreateCubeWire();*/

    /* render a solid cube */
    CreateCubeSolid();

    glFlush();        /* flush all drawing commands */
    glPushMatrix();   /* save the current transformation onto the matrix stack */
    glLoadIdentity(); /* restore modelview to eye coordinates */

    SwapBuffers(hdc); /* swap the front and back window buffers */
  }

void CreateCubeWire(void)
  {
    glColor3f(1.0f, 1.0f, 1.0f); /* draw lines in white color */

    glBegin(GL_LINE_STRIP);  /* front face */
      glVertex3f(-10.0f, 10.0f, 10.0f);
      glVertex3f(10.0f, 10.0f, 10.0f);
      glVertex3f(10.0f, -10.0f, 10.0f);
      glVertex3f(-10.0f, -10.0f, 10.0f);
      glVertex3f(-10.0f, 10.0f, 10.0f);
    glEnd();

    glBegin(GL_LINE_STRIP);  /* back face */
      glVertex3f(-10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, -10.0f);
      glVertex3f(-10.0f, -10.0f, -10.0f);
      glVertex3f(-10.0f, 10.0f, -10.0f);
    glEnd();

    glBegin(GL_LINES);  /* adjoining lines from front to back */
      glVertex3f(-10.0f, 10.0f, 10.0f);
      glVertex3f(-10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, 10.0f, 10.0f);
      glVertex3f(10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, 10.0f);
      glVertex3f(10.0f, -10.0f, -10.0f);
      glVertex3f(-10.0f, -10.0f, 10.0f);
      glVertex3f(-10.0f, -10.0f, -10.0f);
    glEnd();
  }

void CreateCubeSolid(void)
  {
    glBegin(GL_QUADS);  /* front face */
      glColor3f(1.0f, 0.0f, 0.0f); /* RED */
      glVertex3f(-10.0f, 10.0f, 10.0f);
      glVertex3f(10.0f, 10.0f, 10.0f);
      /*glColor3f(1.0f, 1.0f, 1.0f); for smooth shading */
      glVertex3f(10.0f, -10.0f, 10.0f);
      glVertex3f(-10.0f, -10.0f, 10.0f);
    glEnd();

    glBegin(GL_QUADS);  /* back face */
      glColor3f(0.0f, 1.0f, 0.0f); /* GREEN */
      glVertex3f(-10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, -10.0f);
      glVertex3f(-10.0f, -10.0f, -10.0f);
    glEnd();

    glBegin(GL_QUADS);  /* top face */
      glColor3f(0.0f, 0.0f, 1.0f); /* BLUE */
      glVertex3f(-10.0f, 10.0f, 10.0f);
      glVertex3f(-10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, 10.0f, 10.0f);
    glEnd();

    glBegin(GL_QUADS);  /* bottom face */
      glColor3f(1.0f, 1.0f, 0.0f); /* YELLOW */
      glVertex3f(-10.0f, -10.0f, 10.0f);
      glVertex3f(-10.0f, -10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, 10.0f);
    glEnd();

    glBegin(GL_QUADS);  /* left face */
      glColor3f(0.0f, 1.0f, 1.0f); /* AQUA */
      glVertex3f(-10.0f, 10.0f, 10.0f);
      glVertex3f(-10.0f, 10.0f, -10.0f);
      glVertex3f(-10.0f, -10.0f, -10.0f);
      glVertex3f(-10.0f, -10.0f, 10.0f);
    glEnd();

    glBegin(GL_QUADS);  /* right face */
      glColor3f(1.0f, 0.0f, 1.0f); /* PURPLE */
      glVertex3f(10.0f, 10.0f, 10.0f);
      glVertex3f(10.0f, 10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, -10.0f);
      glVertex3f(10.0f, -10.0f, 10.0f);
    glEnd();
  }
