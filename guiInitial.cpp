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


main () {
	XEvent event;		/* the XEvent declaration !!! */
	KeySym key;		/* a dealie-bob to handle KeyPress Events */	
	char text[255];		/* a char buffer for KeyPress Events */
	std::string uname;
	std::string pass;
	
	init_x();
	//get_colors();
    //XSetForeground(dis,gc,chartreuse);
	std::map<int, std::string> data;
	int x=250;
	int n =0;
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
		XDrawString(dis, win, gc, 300, 50, "LOGIN  USER", strlen("LOGIN  USER"));
		XSetLineAttributes(dis, gc, 5, LineDoubleDash, CapNotLast, JoinBevel);
		XDrawLine(dis, win, gc, 0, 100, 800, 100);
		XDrawString(dis, win, gc, 125, 150, "Username", strlen("Username"));
		XDrawString(dis, win, gc, 125, 200, "Password", strlen("Password"));
		XSetLineAttributes(dis, gc, 3, LineSolid, CapNotLast, JoinBevel);
		XDrawLine(dis, win, gc, 250, 150, 600, 150);
		XDrawLine(dis, win, gc, 250, 200, 600, 200);
		XNextEvent(dis, &event);
	
		if (event.type==Expose && event.xexpose.count==0) {
		/* the window was exposed redraw it! */
			redraw();
		}
		if (event.type==KeyPress&&
		    XLookupString(&event.xkey,text,255,&key,0)==1) {
		/* use the XLookupString routine to convert the invent
		   KeyPress data into regular text.  Weird but necessary...
		*/
		
			if (text[0]==13)
			{
				p = 1;
				x = 250;
				pass.erase();
				//std::cout<<p<<std::endl;
			}
			else if (text[0]=='q') {
				close_x();
			}
			else if (text[0]==' ') {
				
				
				if (p==0)
				{
					data.insert(std::make_pair(x, uname));
					x += 10*uname.length();
					uname.clear();
				}
				else
				{
					pass +='-';
				}

				
			}
			else if (text[0]=='\b') {
				if (uname.length()-1 >= 0 && p==0) {
				uname[uname.length()-1] = ' ';
				XClearWindow(dis, win);
				unsigned long black = BlackPixel(dis,screen);
				XSetForeground(dis,gc,black);
				std::map<int, std::string>::iterator iter;
				for(iter = data.begin();iter != data.end(); iter++)
				{
					if (p==0)
					{
						set_up_font();
						XDrawString(dis,win,gc,iter->first,150, (iter->second).c_str(), (iter->second).length());
					}
					

				}
				set_up_font();
				XDrawString(dis,win,gc,x,150, uname.c_str(), uname.length()-1);
				if (uname.length()-1 > 0)
					uname.erase(uname.length()-1);
				else if (uname.length() == 1)
					uname.clear();
				}
				else if(p==1 && pass.length()-1 >=0)
				{
					pos = 250;
					pass[pass.length()-1] = ' ';
					XClearWindow(dis,win);
					std::map<int, std::string>::iterator iter;
					for(iter = data.begin();iter != data.end(); iter++)
					{
						if (p==1)
						{
							set_up_font();
							XDrawString(dis,win,gc,iter->first,150, (iter->second).c_str(), (iter->second).length());
							pos +=10*(iter->second).length();
						}
									

					}
					//std::cout<<pos;
					XDrawString(dis,win,gc,(pos),150, uname.c_str(), uname.length());
					//unsigned long black = BlackPixel(dis,screen);
					//XSetForeground(dis,gc,black);
					XDrawString(dis,win,gc,x,200, pass.c_str(), pass.length()-1);
					if (pass.length()-1 > 0)
					 	pass.erase(pass.length()-1);
					else if (pass.length() == 1)
					 	pass.clear();
				}
			}
			else
			{
				if (p==0)
				uname += text[0];
				else
				pass +='-';
				//std::cout<<"length of pass"<<pass.length()<<std::endl;
			}
			std::cout<<uname<<'\n';
			printf("You pressed the %d key!\n",text[0]);
			//char ptext[200];
			//strcpy(ptext,uname.c_str());
			unsigned long black = BlackPixel(dis,screen);
			XSetForeground(dis,gc,black);
			set_up_font();
			if (p==0)
			XDrawString(dis,win,gc,x,150, uname.c_str(), uname.length());
			else
			XDrawString(dis,win,gc,x,200, pass.c_str(), pass.length());	
		}
		if (event.type==ButtonPress) {
		/* tell where the mouse Button was Pressed */
			int x=event.xbutton.x,
			    y=event.xbutton.y;

			strcpy(text,"X is FUN!");
			XSetForeground(dis,gc,rand()%event.xbutton.x%255);
			XDrawString(dis,win,gc,x,y, text, strlen(text));
			//XDrawRectangle(dis, win, gc, 10, 10, 50, 20);
			//XDrawLine(dis, win, gc, 10, 10, 50, 50);
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
		800, 500, 10 ,black, white);
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
	exit(1);				
};

void redraw() {
	XClearWindow(dis, win);
};
