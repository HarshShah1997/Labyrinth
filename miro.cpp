#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <cstring>
#include <SOIL.h>

#include "miro.h"
//#include "pathfinder.h"

bool loadTexture(char *path, GLuint *texture){
    *texture = SOIL_load_OGL_texture(path,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_MULTIPLY_ALPHA
            );
    if(*texture == 0){
        return false;
    } else {
        return true;
    }
}


class PathFinder {
private:
	/* position and direction for path finding */
	double current_x;
	double current_y;
	double old_x;
	double old_y;
	int Dest;
	int init_dest;
        int walk_status;
        GLuint textureId;

	bool ismoving;
	double degree_7;	// a degree of 7, used calculating movement of one frame of animation
	double bodyColorR;
	double bodyColorG;
	double bodyColorB;


public:
	enum Direction { UP = 0, DOWN = 1, RIGHT = 2, LEFT = 3 };

	void SetBodyColor(double r, double g, double b)	{ bodyColorR = r; bodyColorG = g, bodyColorB = b; }

	double CurrentX() { return current_x; }	// return x position of the finder
	double CurrentY() { return current_y; }	// return y position of the finder
        bool isMoving() { return ismoving; } 


void set_dest( Direction new_dest ){
    this->init_dest = Dest;
    this->Dest = new_dest;
    ismoving = true;
}

PathFinder(int x_position, int y_position, int maze_width, int maze_height)
{
    old_x = current_x = 20.0 + 10.0 * x_position;
    old_y = current_y = 20.0 + 10.0 * y_position;

    /* initialzing status factor */
    ismoving = false;

    degree_7 = sin(7 * atan(-1) / 180);	// sin( 7 * PI / 180)
    textureId = 20;
    if (!loadTexture("textures/Earth.png", &textureId)) {
        std::cout << "not found" << std::endl;
    }

    init_dest = Dest = RIGHT;
}

void Move()
{
    double movingfactor = 20 * fabs(sin(degree_7 * walk_status) - sin(degree_7 * (walk_status - 1)));
    // movement of one frame of the animation

    switch(Dest) {
        case UP:
            current_y += movingfactor;
            break;
        case DOWN:
            current_y -= movingfactor;			
            break;
        case LEFT:
            current_x -= movingfactor;
            break;
        case RIGHT:
            current_x += movingfactor;
            break;
    }

    if (abs(old_x - current_x) >= 10.0) {
        current_x = old_x + ((Dest == RIGHT) ? 10 : -10);
        old_x = current_x;
        ismoving = false;
    } else if (abs(old_y - current_y) >= 10.0) {
        current_y = old_y + ((Dest == UP) ? 10 : -10);
        old_y = current_y;
        ismoving = false;
    }
}

void Draw()
{
    glTranslatef( 30, 50, 0 );

    // draw body
    glTranslatef(20,15,0);
    glTranslatef(-20, -15, 0);
    glColor3f( bodyColorR, bodyColorG, bodyColorB );
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glBegin( GL_POLYGON );
    glEdgeFlag( GL_TRUE );
    for (float angle = 0; angle < 2 * M_PI; angle = angle + 0.01) {
        glTexCoord2f(0.5 * cos(angle) + 0.5, 0.5 * sin(angle) + 0.5);
        glVertex2f(20 * cos(angle) + 20, 20 * sin(angle) + 20);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

}

void UpdateStatus() {
    if (ismoving) Move();
}

};


void display();
void reviewpoint();
void RenderString(float x, float y, void *font, const unsigned char* msg, float r, float g, float b);
void display_win();

static int view_Left, view_Right, view_Bottom, view_Up;	//view points
static int ViewZoomFactor;
static int ViewChange_x, ViewChange_y;
static int timefactor;		// controls duration

static Cell *cell;
static int width, height;	// the size of maze
static int starting_x, starting_y;	// starting position
static int goal_x, goal_y;	// position of goal
static double R, G, B;		// the color of background
static int *chosen;		// the pointer of array of connected cells
static bool work;			// making maze(true) or pause(false)
static int state = 0;		// current state(making maze, finding path or end)
static PathFinder* gb_finder = NULL;	// path finder object
static bool Over_view = true;
static int userInputLastDirection = -1;

static inline Cell & cellXY(int x, int y) {
    return cell[y * width + x];
}


void RenderString(float x, float y, void *font, const unsigned char* msg, float r, float g, float b)
{  
    char *c;

    glColor3f(r, g, b); 
    glRasterPos2f(x, y);

    glutBitmapString(font, msg);
}

// remove the line between two connected cells
void erase_wall( int x, int y, int dest ){


    glColor3f( 0, 0, 0 );
    glBegin( GL_LINES );

    switch( dest ){

        case up:
            glVertex2f( (x+1)*10+0.02, (y+2)*10 );
            glVertex2f( (x+2)*10-0.02, (y+2)*10 );
            break;

        case down:
            glVertex2f( (x+1)*10+0.02, (y+1)*10 );
            glVertex2f( (x+2)*10-0.02, (y+1)*10 );
            break;

        case right:
            glVertex2f( (x+2)*10, (y+1)*10+0.02 );
            glVertex2f( (x+2)*10, (y+2)*10-0.02 );
            break;

        case left:
            glVertex2f( (x+1)*10, (y+1)*10+0.02 );
            glVertex2f( (x+1)*10, (y+2)*10-0.02 );
            break;
    }

    glEnd();
}

void draw_maze() {

    int i;
    int x, y;

    for( i = 0 ; i < width*height ; i++ ){
        x = i % width;
        y = i / width;

        if( cell[i].road[right] == true )	erase_wall( x, y, right );
        if( cell[i].road[up] ==true )		erase_wall( x, y, up );
        if( cell[i].road[down] == true )	erase_wall( x, y, down );
        if( cell[i].road[left] ==true )		erase_wall( x, y, left );
    }
}

void choose_starting()
{
    int x, y;
    int dest = rand()%2 + 1;
    if( dest == down ){
        
        starting_x = x = rand()%width;
        starting_y = y = height - 1;
        cellXY(x, y).road[up] = true;
        
        goal_x = x = rand()%width;
        goal_y = y = 0;
        cellXY(x, y).road[down] = true;
    } else {
        starting_x = x = width - 1;
        starting_y = y = rand()%height;
        cellXY(x, y).road[right] = true;

        goal_x = x = 0;
        goal_y = y = rand()%height;
        cellXY(x, y).road[left] = true;
    }
    chosen = new int [height * width];

    // choose the first cell randomly
    x = rand()%width;
    y = rand()%height;
    cellXY(x, y).is_open = true;
    chosen[0] = width*y + x;	
}

// generate the maze
void gen_maze(){

    int x, y;	// position of the current cell
    int dest;	// direction of to be connected cell
    static int length = 0;	
    int tmp;

    //if( work == false ) return;
    if( length == width * height) {
        state = 1;
        for(int i = 0; i < width*height; i++)
            cell[i].is_open = false;
        return;
    }


    if( length == 0 ){
        choose_starting();
        length = 1;
    }

    bool cellOpen = false;
    int counter = 0;
    while (!cellOpen) {
        tmp = chosen[ rand()%length ];	
        x = tmp % width;
        y = tmp / width;

        dest = rand()%4;	
        counter++;
        switch( dest ){

            case up:
                if( y == height-1 || cellXY(x, y + 1).is_open == true)
                    continue;

                //std::cout << counter << " " << length  << " " << x << "," << y  << " -> " << x << "," << y+1 << std::endl;
                cellXY(x, y + 1).is_open = true;

                cellXY(x, y + 1).road[ down ] = true;
                cellXY(x, y).road[ up ] = true;

                chosen[length] = width*( y + 1 ) + x;
                length++;
                cellOpen = true;
                break;

            case down:
                if( y == 0 || cellXY(x, y - 1).is_open == true )
                    continue;

                //std::cout << counter << " " << length  << " " << x << "," << y  << " -> " << x << "," << y-1 << std::endl;
                cellXY(x, y - 1).is_open = true;

                cellXY(x, y - 1).road[ up ] = true;
                cellXY(x, y).road[ down ] = true;

                chosen[length] = width*(y - 1) + x;
                length++;
                cellOpen = true;
                break;

            case right:
                if( x == width-1 || cellXY(x + 1, y).is_open == true )
                    continue;

                //std::cout << counter << " " << length  << " " << x << "," << y  << " -> " << x+1 << "," << y << std::endl;
                cellXY(x + 1,  y).is_open = true;

                cellXY(x + 1,  y).road[ left ] = true;
                cellXY(x,  y).road[ right ] = true;

                chosen[length] = width * y + x + 1;
                length++;
                cellOpen = true;
                break;

            case left:
                if( x == 0 || cellXY(x - 1,  y).is_open == true )
                    continue;

                cellXY(x - 1,  y).is_open = true;

                cellXY(x - 1,  y).road[ right ] = true;
                cellXY(x,  y).road[ left ] = true;

                chosen[length] = width * y + x - 1;
                length++;
                cellOpen = true;
                break;
        }
    }
}

void init() {
    int size = ( width > height )? width : height;
    double move = ( width > height )? ( width-height )/2.0 : ( height-width )/2.0;

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();

    if( width == size ){
        view_Left = 0.0;
        view_Right = 20 + size*10;
        view_Bottom = 0.0 - move*10;
        view_Up = size*10 + 20 - move*10;
    }  else {
        view_Left = 0.0 - move*10;
        view_Right = 20 + size*10 - move*10;
        view_Bottom = 0.0;
        view_Up = size*10 + 20;
    }

    gluOrtho2D( view_Left, view_Right, view_Bottom, view_Up );

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

void draw_texture()
{
    GLuint id = 13;
    if (!loadTexture("textures/celestial002.png", &id)) {
        std::cout << "Texture not found\n";
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(view_Left, view_Bottom);

        glTexCoord2f(1, 0);
        glVertex2f(view_Right, view_Bottom);

        glTexCoord2f(1, 1);
        glVertex2f(view_Right, view_Up);

        glTexCoord2f(0, 1);
        glVertex2d(view_Left, view_Up);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


void display()
{
    double x;

    glClearColor( R, G, B, 0.0 );
    glClear( GL_COLOR_BUFFER_BIT );
    draw_texture();

    glColor3f( 1.0, 1.0, 1.0 );	

    // draw default(unmaden) maze
    glLoadIdentity();
    glBegin( GL_LINES );
    for( x = 1 ; x < width+2 ; x++ ){
        glVertex2f( x*10, 10.0 );
        glVertex2f( x*10, height*10+10.0 );
    }
    for( x = 1 ; x < height+2; x++ ){
        glVertex2f( 10.0 , x*10 );
        glVertex2f( width*10+10.0 , x*10 );
    }
    glEnd();
    
    draw_maze();

    if(gb_finder != NULL) {
        const double SHIFTFACTOR_X = -10.0;
        const double SHIFTFACTOR_Y = -11.5;

        glLoadIdentity();
        glTranslatef(gb_finder->CurrentX() + SHIFTFACTOR_X, gb_finder->CurrentY() + SHIFTFACTOR_Y, 0);
        glScalef(0.1, 0.1, 1);
        gb_finder->Draw();
    }

    glLoadIdentity();

    if (state == 2) {
        display_win();
    }

    glutSwapBuffers();
}

// the space bar toggles making maze or pause
void keyFunc( unsigned char key, int x, int y ){
    switch (key) {
        case 13:
            work = !work;
            break;
    }
}

void path_finding()
{
    static PathFinder finder(::starting_x, ::starting_y, ::width, ::height);
    static int x = ::starting_x;
    static int y = ::starting_y;

    if (gb_finder == NULL) {
        gb_finder = &finder;	// to use in other functions
        finder.SetBodyColor(1.0, 1.0, 1.0);
    }

    finder.UpdateStatus();
    if(finder.isMoving()) {
        return;
    }

    if(x == ::goal_x && y == ::goal_y) {	// if get the goal
        ::state++;
        return;
    }

    if (userInputLastDirection > -1) {
        switch (userInputLastDirection) {
            case up:
                if(cellXY(x, y).road[up] == true && y < ::height-1 ) {
                    finder.set_dest(PathFinder::UP);
                    y++;
                }
                break;
            case down:
                if(cellXY(x,  y).road[down] == true && y > 0 ) {
                    finder.set_dest(PathFinder::DOWN);
                    y--;
                }
                break;
            case right:
                if(cellXY(x, y).road[right] == true && x < ::width-1 ) {
                    finder.set_dest(PathFinder::RIGHT);
                    x++;
                }
                break;
            case left:
                if(cellXY(x, y).road[left] == true && x > 0 ) {
                    finder.set_dest(PathFinder::LEFT);
                    x--;
                }
                break;
        }
        userInputLastDirection = -1;
    }
}

void display_win()
{
    draw_texture();
    float x = (view_Right + view_Left) / 2.0 - 7;
    float y = (view_Up + view_Bottom) / 2.0;
    
    RenderString(x, y, GLUT_BITMAP_TIMES_ROMAN_24, "You Win!!!", 153.0/255, 204.0/255, 1.0);
}

void goal_ceremony()
{
    static int count = 0;
    count++;
    glLoadIdentity();

    if( count > 100) state++;
}

void specialKeyFunc( int key, int x, int y ){
    switch (key) {
        case GLUT_KEY_RIGHT:
            userInputLastDirection = right;
            break;
        case GLUT_KEY_LEFT:
            userInputLastDirection = left;
            break;
        case GLUT_KEY_DOWN:
            userInputLastDirection = down;
            break;
        case GLUT_KEY_UP:
            userInputLastDirection = up;
            break;
    }

    display();
}

void idle()
{
    if( work == false ) return;
    switch (state) {
        case 0:
            gen_maze();
            break;
        case 1:
            path_finding();
            break;
        case 2:
            goal_ceremony();
            break;
        case 3:
            exit(0);
            break;
    }
    display();
}

int main( int argc, char ** argv ){
    using namespace std;

    srand( ( unsigned )time( NULL ) );

    /* input the size of the maze */
    while(1) {
        cout<<"Input width, height (From 5 to 30)"<<endl;
        cin >> width >> height;
        if( width > 30 || width < 5 || height > 30 || height < 5) {
            cout << "out of range!" << endl;
        } else {
            break;
        }
    }

    /* help message */
    cout << endl;
    cout << "Enter key : start/stop working" << endl;
    cout << "Arrow key : move the character" << endl;
    cout << endl << "SOLVE THE MAZE!!!" << endl;

    cell = new Cell[width * height];

    R = 0.0;
    G = 0.0;
    B = 0.0;

    timefactor = INIT_TIMEFACTOR;
    ViewChange_x = 0;
    ViewChange_y = 0;

    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB );
    glutInitWindowSize ( 700, 700 );
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Labyrinth");

    init();

    glutDisplayFunc( display );
    glutIdleFunc( idle );
    glutSpecialFunc( specialKeyFunc );
    glutKeyboardFunc( keyFunc );

    glutMainLoop();

    return 0;
}
