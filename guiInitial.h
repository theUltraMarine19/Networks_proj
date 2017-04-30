/* 
   Remember to compile try:
   1) gcc hi.c -o hi -lX11
   2) gcc hi.c -I /usr/include/X11 -L /usr/X11/lib -lX11
   3) gcc hi.c -I /where/ever -L /who/knows/where -l X11

   Brian Hammond 2/9/96.    Feel free to do with this as you will!
   */


/* include the X library headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xos.h>

/* include some silly stuff */
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <utility>

// struct Button {
// 	XChar2b * text;
// 	int text_width;
// 	int font_ascent;
// 	int width, height;
// 	unsigned long border, background, foreground;
// 	void *cbdata;
// 	Callback buttonRelease;
// };


/* here are our X variables */
Display *dis;
int screen;
Window win;
GC gc;
long int chartreuse;
XFontStruct * font;

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();
void get_colors();
static void set_up_font();
static void set_up_font2();

class Position
{
    public:
    int x;
    int y;
    Position(int x_, int y_)
    {
        x = x_;
        y = y_;
    }
};
int loginPage(std::string &uname, std::string &password, std::string msg)
{
    XEvent event;		/* the XEvent declaration !!! */
    KeySym key;		/* a dealie-bob to handle KeyPress Events */
    char text[255];		/* a char buffer for KeyPress Events */
    std::string pass;
    bool done=false;

    std::map<int, std::string> data;
    std::map<int, std::string> dataPass;
    int yUsername = 350;
    int yPassword = 450;
    int xUsername = 500;
    int xPassword = 500;
    int yButton = 550;
    int loginX = 500, loginY=yButton, loginH=50, loginW=100;
    int registerX = 700, registerY=yButton, registerH=50, registerW=100;
    std::string currText;
    int n = 0;
    int p = 0;
    int pos = 250;
    /* look for events forever... */
    while(1) {
        /* get the next event and stuff it into our event variable.
Note:  only events we set the mask for are detected!
*/
        //XDrawRectangle(dis, win, gc, 10, 10, 50, 20);

        set_up_font2();
        //Button *button = NULL;
        XSetLineAttributes(dis, gc, 5, LineDoubleDash, CapNotLast, JoinBevel);
        XDrawLine(dis, win, gc, 0, 30, 1300, 30);
        XDrawString(dis, win, gc, 550, 100, "LOGIN  USER", strlen("LOGIN  USER"));
        XSetLineAttributes(dis, gc, 5, LineDoubleDash, CapNotLast, JoinBevel);
        XDrawLine(dis, win, gc, 0, 150, 1300, 150);
        XDrawString(dis, win, gc, 400, 350, "Username", strlen("Username"));
        XDrawString(dis, win, gc, 400, 450, "Password", strlen("Password"));
        XSetLineAttributes(dis, gc, 3, LineSolid, CapNotLast, JoinBevel);
        XDrawLine(dis, win, gc, 500, yUsername, 800, yUsername);
        XDrawLine(dis, win, gc, 500, yPassword, 800, yPassword);
        XDrawRectangle(dis, win, gc, 500, yButton, 100, 50);
        XDrawString(dis, win, gc, 520, yButton+30, "Login", strlen("Login"));
        XDrawRectangle(dis, win, gc, 700, yButton, 100, 50);
        XDrawString(dis, win, gc, 710, yButton+30, "Register", strlen("Register"));
        XDrawString(dis, win, gc, 100, yButton+100, msg.c_str(), msg.length());
        XNextEvent(dis, &event);
        set_up_font();

        if (event.type==Expose && event.xexpose.count==0) {
            /* the window was exposed redraw it! */
            redraw();
        }
        if (event.type==KeyPress&&
                XLookupString(&event.xkey,text,255,&key,0)==1) {
            /* use the XLookupString routine to convert the invent
               KeyPress data into regular text.  Weird but necessary...
               */

            if (text[0]==13) {
                if (p==0)
                {
                    data.insert(std::make_pair(xUsername, currText));
                    uname = currText;
                    currText.clear();
                    p = 1;
                }
                else if(p==1)
                {
                    dataPass.insert(std::make_pair(yUsername, currText));
                    if(!done)
                        password = currText;
                    currText.clear();
                    done = true;
                }

            }
            else if (text[0]=='\b') {
                if (currText.length()-1 >= 0 && p==0) {
                    currText[currText.length()-1] = ' ';
                    XClearWindow(dis, win);
                    unsigned long black = BlackPixel(dis,screen);
                    XSetForeground(dis,gc,black);
                    std::map<int, std::string>::iterator iter;
                    for(iter = data.begin();iter != data.end(); iter++)
                    {
                        set_up_font();
                        XDrawString(dis,win,gc,iter->first,yUsername, (iter->second).c_str(), (iter->second).length());
                    }
                    set_up_font();
                    XDrawString(dis,win,gc,xUsername,yUsername, currText.c_str(), currText.length()-1);
                    if (currText.length()-1 > 0)
                        currText.erase(currText.length()-1);
                    else if (currText.length() == 1)
                        currText.clear();
                }
                else if(p==1 && currText.length()-1 >=0)
                {
                    currText[currText.length()-1] = ' ';
                    pass[pass.length()-1] = ' ';
                    XClearWindow(dis,win);
                    std::map<int, std::string>::iterator iter;
                    for(iter = dataPass.begin();iter != dataPass.end(); iter++)
                    {
                        set_up_font();
                        XDrawString(dis,win,gc,iter->first,yPassword, (iter->second).c_str(), (iter->second).length());
                    }
                    XDrawString(dis,win,gc,xUsername,yUsername, uname.c_str(), uname.length());
                    XDrawString(dis,win,gc,xPassword, yPassword, pass.c_str(), pass.length()-1);
                    if (pass.length()-1 > 0) {
                        pass.erase(pass.length()-1);
                        currText.erase(currText.length()-1);
                    }
                    else if (pass.length() == 1) {
                        pass.clear();
                        currText.clear();
                    }
                }
            }
            else if(isalnum(text[0]))
            {
                if (p==1)
                    pass +='*';
                currText += text[0];
                //std::cout<<"length of pass"<<pass.length()<<std::endl;
            }
            std::cout<<"p = "<<p<<", currText = "<<currText<<'\n';
            printf("You pressed the %d key!\n",text[0]);
            unsigned long black = BlackPixel(dis,screen);
            XSetForeground(dis,gc,black);
            set_up_font();
            if (p==0)
                XDrawString(dis,win,gc,xUsername,yUsername, currText.c_str(), currText.length());
            else
                XDrawString(dis,win,gc,xPassword,yPassword, pass.c_str(), pass.length());
        }
        if (event.type==ButtonPress) {
            /* tell where the mouse Button was Pressed */
            int x=event.xbutton.x,
                y=event.xbutton.y;

            //XSetForeground(dis,gc,255);
            //XDrawString(dis,win,gc,x,y, text, strlen(text));
            //XDrawRectangle(dis, win, gc, 10, 10, 50, 20);
            //XDrawLine(dis, win, gc, 10, 10, 50, 50);
            if (!done)
                password = currText;
            if (x > loginX and x < (loginX + loginW) and
                y > loginY and y < (loginY + loginH))
               return 2;
            else if (x > registerX and x < (registerX + registerW) and
                y > registerY and y < (registerY + registerH))
               return 1;
        }
    }
}

void get_colors(){
    XColor tmp;
    XParseColor(dis, DefaultColormap(dis,screen), "chartreuse", &tmp);
    XAllocColor(dis,DefaultColormap(dis,screen),&tmp);
    chartreuse=tmp.pixel;
};


void init_x() {
    /* get the colors black and white (see section for details) */
    unsigned long black,white;

    dis=XOpenDisplay((char *)0);
    screen=DefaultScreen(dis);
    black=BlackPixel(dis,screen),
        white=WhitePixel(dis, screen);
    win=XCreateSimpleWindow(dis,DefaultRootWindow(dis),0,0,	
            1600, 1000, 10 ,black, white);
    XSetStandardProperties(dis,win,"Login Window","Hi",None,NULL,0,NULL);
    XSelectInput(dis, win, ExposureMask|ButtonPressMask|KeyPressMask);
    gc=XCreateGC(dis, win, 0,0);
    //draw_graphics(win, gc, 200, 300);
    XSetBackground(dis,gc,white);
    XSetForeground(dis,gc,black);
    XClearWindow(dis, win);
    XMapWindow(dis, win);


};

static void set_up_font ()
{
    const char * fontname = "-*-helvetica-*-r-*-*-20-*-*-*-*-*-*-*";
    font = XLoadQueryFont (dis, fontname);
    /* If the font could not be loaded, revert to the "fixed" font. */
    if (! font) {
        fprintf (stderr, "unable to load font %s: using fixed\n", fontname);
        font = XLoadQueryFont (dis, "fixed");
    }
    XSetFont (dis, gc, font->fid);
};

static void set_up_font2 ()
{
    const char * fontname = "-*-helvetica-*-r-*-*-24-*-*-*-*-*-*-*";
    font = XLoadQueryFont (dis, fontname);
    /* If the font could not be loaded, revert to the "fixed" font. */
    if (! font) {
        fprintf (stderr, "unable to load font %s: using fixed\n", fontname);
        font = XLoadQueryFont (dis, "fixed");
    }
    XSetFont (dis, gc, font->fid);
};



void close_x() {
    XFreeGC(dis, gc);
    XDestroyWindow(dis,win);
    XCloseDisplay(dis);
};

void redraw() {
    XClearWindow(dis, win);
};
