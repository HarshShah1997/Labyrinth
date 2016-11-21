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

#define	UP	0
#define	DOWN	1
#define RIGHT	2
#define LEFT	3

using namespace std;
class GridBox {
    public:
        bool isVisited;	
        bool pathWay[4];	

        GridBox() {
            isVisited = false;

            for (int i = 0; i < 4; i++) {
                pathWay[i] = false;
            }
        }
}; 

class Ball {
    private:
        double current_x;
        double current_y;
        double old_x;
        double old_y;
        int Dest;
        int init_dest;
        GLuint textureId;

        bool ismoving;
        double bodyColorR;
        double bodyColorG;
        double bodyColorB;


    public:

        double CurrentX() { 
            return current_x; 
        }	
        double CurrentY() { 
            return current_y;
        }	
        bool isMoving() {
            return ismoving; 
        } 

        bool loadTexture(char *filename, GLuint *texture){
            *texture = SOIL_load_OGL_texture(filename,
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

        void fix_destination( int new_dest ){
            this->init_dest = Dest;
            this->Dest = new_dest;
            ismoving = true;
        }

        Ball(int x_position, int y_position, int maze_width, int maze_height)
        {
            bodyColorR = 1.0;
            bodyColorG = 1.0;
            bodyColorB = 1.0;
            old_x = current_x = 20.0 + 10.0 * x_position;
            old_y = current_y = 20.0 + 10.0 * y_position;

            ismoving = false;

            textureId = 20;
            if (!loadTexture("textures/Earth.png", &textureId)) {
                cout << "not found" << endl;
            }

            init_dest = Dest = RIGHT;
        }

        void Move()
        {
            double movingfactor = 20 * 0.0305385;

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

        void DrawBall()
        {
            glTranslatef( 30, 50, 0 );

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
};

void display();
void RenderString(float x, float y, void *font, const unsigned char* msg, float r, float g, float b);
void display_win();

int view_Left, view_Right, view_Bottom, view_Up;	

GridBox *grid;
int width, height;	
int init_x, init_y;
int goal_x, goal_y;	
double R, G, B;		
int *chosen;	
bool isWorking;		
int state = 0;		
Ball* ball = NULL;
int userInputLastDirection = -1;

GridBox & getPosXY(int x, int y) {
    return grid[y * width + x];
}


void RenderString(float x, float y, void *font, const unsigned char* msg, float r, float g, float b)
{  
    glColor3f(r, g, b); 
    glRasterPos2f(x, y);

    glutBitmapString(font, msg);
}

void remove_line( int x, int y, int dest ){


    glColor3f( 0, 0, 0 );
    glBegin( GL_LINES );

    switch( dest ){

        case UP:
            glVertex2f( (x+1)*10+0.02, (y+2)*10 );
            glVertex2f( (x+2)*10-0.02, (y+2)*10 );
            break;

        case DOWN:
            glVertex2f( (x+1)*10+0.02, (y+1)*10 );
            glVertex2f( (x+2)*10-0.02, (y+1)*10 );
            break;

        case RIGHT:
            glVertex2f( (x+2)*10, (y+1)*10+0.02 );
            glVertex2f( (x+2)*10, (y+2)*10-0.02 );
            break;

        case LEFT:
            glVertex2f( (x+1)*10, (y+1)*10+0.02 );
            glVertex2f( (x+1)*10, (y+2)*10-0.02 );
            break;
    }

    glEnd();
}

void update_labyrinth() {

    int i;
    int x, y;

    for( i = 0 ; i < width*height ; i++ ){
        x = i % width;
        y = i / width;

        if (grid[i].pathWay[RIGHT] == true) {
            remove_line( x, y, RIGHT );
        }
        if (grid[i].pathWay[UP] == true) {
            remove_line( x, y, UP );
        }
        if (grid[i].pathWay[DOWN] == true) {
            remove_line( x, y, DOWN );
        }
        if (grid[i].pathWay[LEFT] == true) {
            remove_line( x, y, LEFT );
        }
    }
}

void choose_starting()
{
    int x, y;
    int dest = rand()%2 + 1;
    if( dest == DOWN ){

        init_x = x = rand()%width;
        init_y = y = height - 1;
        getPosXY(x, y).pathWay[UP] = true;

        goal_x = x = rand()%width;
        goal_y = y = 0;
        getPosXY(x, y).pathWay[DOWN] = true;
    } else {
        init_x = x = width - 1;
        init_y = y = rand()%height;
        getPosXY(x, y).pathWay[RIGHT] = true;

        goal_x = x = 0;
        goal_y = y = rand()%height;
        getPosXY(x, y).pathWay[LEFT] = true;
    }
    chosen = new int [height * width];

    x = rand()%width;
    y = rand()%height;
    getPosXY(x, y).isVisited = true;
    chosen[0] = width*y + x;	
}

void gen_maze(){

    int x, y;	// position of the current grid
    int dest;	// direction of to be connected grid
    static int length = 0;	
    int tmp;

    if( length == width * height) {
        state = 1;
        for(int i = 0; i < width*height; i++)
            grid[i].isVisited = false;
        return;
    }


    if( length == 0 ){
        choose_starting();
        length = 1;
    }

    bool gridOpen = false;
    int counter = 0;
    while (!gridOpen) {
        tmp = chosen[ rand()%length ];	
        x = tmp % width;
        y = tmp / width;

        dest = rand()%4;	
        counter++;
        switch( dest ){

            case UP:
                if( y == height-1 || getPosXY(x, y + 1).isVisited == true)
                    continue;

                getPosXY(x, y + 1).isVisited = true;

                getPosXY(x, y + 1).pathWay[ DOWN ] = true;
                getPosXY(x, y).pathWay[ UP ] = true;

                chosen[length] = width*( y + 1 ) + x;
                length++;
                gridOpen = true;
                break;

            case DOWN:
                if( y == 0 || getPosXY(x, y - 1).isVisited == true )
                    continue;

                getPosXY(x, y - 1).isVisited = true;

                getPosXY(x, y - 1).pathWay[ UP ] = true;
                getPosXY(x, y).pathWay[ DOWN ] = true;

                chosen[length] = width*(y - 1) + x;
                length++;
                gridOpen = true;
                break;

            case RIGHT:
                if( x == width-1 || getPosXY(x + 1, y).isVisited == true )
                    continue;

                getPosXY(x + 1,  y).isVisited = true;

                getPosXY(x + 1,  y).pathWay[ LEFT ] = true;
                getPosXY(x,  y).pathWay[ RIGHT ] = true;

                chosen[length] = width * y + x + 1;
                length++;
                gridOpen = true;
                break;

            case LEFT:
                if( x == 0 || getPosXY(x - 1,  y).isVisited == true )
                    continue;

                getPosXY(x - 1,  y).isVisited = true;

                getPosXY(x - 1,  y).pathWay[ RIGHT ] = true;
                getPosXY(x,  y).pathWay[ LEFT ] = true;

                chosen[length] = width * y + x - 1;
                length++;
                gridOpen = true;
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
    if (!ball -> loadTexture("textures/celestial002.png", &id)) {
        cout << "Texture not found\n";
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

    update_labyrinth();

    if(ball != NULL) {
        double shiftx = -10.0;
        double shifty = -11.5;

        glLoadIdentity();
        glTranslatef(ball->CurrentX() + shiftx, ball->CurrentY() + shifty, 0);
        glScalef(0.1, 0.1, 1);
        ball->DrawBall();
    }

    glLoadIdentity();

    if (state == 2) {
        display_win();
    }

    glFlush();
}

void input_enter_key( unsigned char key, int x, int y ){
    switch (key) {
        case 13:
            isWorking = !isWorking;
            break;
    }
}

void process_input()
{
    static Ball finder(init_x, init_y, width, height);
    static int x = init_x;
    static int y = init_y;

    if (ball == NULL) {
        ball = &finder;	
    }

    if(finder.isMoving()) {
        finder.Move();
        return;
    }

    if(x == goal_x && y == goal_y) {	
        state++;
        return;
    }

    if (userInputLastDirection > -1) {
        switch (userInputLastDirection) {
            case UP:
                if(getPosXY(x, y).pathWay[UP] == true && y < height-1 ) {
                    finder.fix_destination(UP);
                    y++;
                }
                break;
            case DOWN:
                if(getPosXY(x,  y).pathWay[DOWN] == true && y > 0 ) {
                    finder.fix_destination(DOWN);
                    y--;
                }
                break;
            case RIGHT:
                if(getPosXY(x, y).pathWay[RIGHT] == true && x < width-1 ) {
                    finder.fix_destination(RIGHT);
                    x++;
                }
                break;
            case LEFT:
                if(getPosXY(x, y).pathWay[LEFT] == true && x > 0 ) {
                    finder.fix_destination(LEFT);
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

void wait_for_finish()
{
    static int count = 0;
    count++;
    glLoadIdentity();

    if( count > 100) state++;
}

void input_arrow_keys( int key, int x, int y ){
    switch (key) {
        case GLUT_KEY_RIGHT:
            userInputLastDirection = RIGHT;
            break;
        case GLUT_KEY_LEFT:
            userInputLastDirection = LEFT;
            break;
        case GLUT_KEY_DOWN:
            userInputLastDirection = DOWN;
            break;
        case GLUT_KEY_UP:
            userInputLastDirection = UP;
            break;
    }

    display();
}

void background_process()
{
    if( isWorking == false ) return;
    switch (state) {
        case 0:
            gen_maze();
            break;
        case 1:
            process_input();
            break;
        case 2:
            wait_for_finish();
            break;
        case 3:
            exit(0);
            break;
    }
    display();
}

int main( int argc, char ** argv ){

    srand( ( unsigned )time( NULL ) );

    while(1) {
        cout<<"Input width, height (From 5 to 30)"<<endl;
        cin >> width >> height;
        if( width > 30 || width < 5 || height > 30 || height < 5) {
            cout << "out of range!" << endl;
        } else {
            break;
        }
    }

    cout << endl;
    cout << "Enter key : start/stop isWorkinging" << endl;
    cout << "Arrow key : move the character" << endl;
    cout << endl << "SOLVE THE MAZE!!!" << endl;

    grid = new GridBox[width * height];

    R = 0.0;
    G = 0.0;
    B = 0.0;


    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize ( 700, 700 );
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Labyrinth");

    init();

    glutDisplayFunc( display );
    glutIdleFunc( background_process );
    glutSpecialFunc( input_arrow_keys );
    glutKeyboardFunc( input_enter_key );

    glutMainLoop();

    return 0;
}
