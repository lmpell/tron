/*
 * ECE 282 project 2
 * by Luke Pell, Nathan Schwartz, Sami Uzzama
 *Tron.c is the terminal base game of tron
 */

#include	<stdio.h>
#include	<curses.h>
#include	<signal.h>
#include	<aio.h>
#include	<string.h>
#include	<wchar.h>

/* The state of the game 			*/
//For Player one

int	tail= 'o';
char	head ='>';
int	row   = 10;	/* current row		*/
int	col   =  10;	/* current column	*/
int	updown   =  0;	/* where we are going	*/
int	sidetoside =  1;
int	score = 0;	//keeps track of score
FILE    *fileP1;


int	delay = 100;	/* how long to wait	*/
int	done  = 0;

//Player 2 variables  
int	tail1 = '+';
char	head1 ='>';
int	row1   =0 ;	/* current row		*/
int	col1   = 0 ;	/* current column	*/
int	updown1   =  0;	/* where we are going	*/
int	sidetoside1 =  0;
int	score1 = 0;	//keeps track of score
FILE    *fileP2;

//checking variables 
int     Flag =0;
int     Checker =1;

//highscore var
int	highscore=0;


struct aiocb kbcbuf;	/* an aio control buf   */

//prototypes
void  removeplayer(int, int, int, int);
void	printborder(void);

int main(void)
{
	void	on_alarm(int);	/* handler for alarm	*/
	void	on_input(int);	/* handler for keybd    */
    	void	setup_aio_buffer();/* sets up event for keyboard handling*/
	int     value =  0;

	initscr();		/* set up screen */
	start_color();		/* intializes color*/
	crmode();		/*Puts terminal into normal output mode */
	noecho();		/*turns off echo mode*/
	clear();		
	
	curs_set(0);		//turns off the blinking cursor

	init_pair(1, COLOR_RED, COLOR_BLACK); //sets color to red 
	init_pair(2, COLOR_GREEN, COLOR_BLACK);//sets color to green 
	init_pair(3, COLOR_CYAN, COLOR_BLACK);//sets color to cyan

	//sets the colors for the tails of the players
	tail = 'o'|COLOR_PAIR(1);
	tail1= '+'|COLOR_PAIR(2);
    
	//enables interrupt for keyboard inputs
	signal(SIGIO, on_input);          /* install a handler        */
	setup_aio_buffer();	  	  /* initialize aio ctrl buff */
	aio_read(&kbcbuf);		  /* place a read request     */

	//checks to see if there is a player1 or not
	//if true it will set Flag high
	if( fileP1 = fopen("moveP1.txt", "r") ){
	    Flag =1;	    
	}
	
	//opening player files
	fileP1 = fopen("moveP1.txt", "w+");
	fileP2 = fopen("moveP2.txt", "w+");

	//setting permissions for different users to play the game 
	chmod("moveP1.txt",0777);
	chmod("moveP2.txt",0777);

	//turning on the alarm interrupt
	signal(SIGALRM, on_alarm);        /* install alarm handler    */
	set_ticker(delay);	 /* start ticking	      */

	//printing head of first playabale character 
    	mvaddch( row, col, head);    /* draw initial image       */
	
	//prints the border
	printborder();
	refresh();
	
	while( !done )			  /* the main loop */
	    pause();

	//closes the window 
	endwin();
    
	//removing files from directory, 
	//to prevent errors when rerunning exectuable 
	remove("moveP1.txt");
	remove("moveP2.txt");

	//closes files 
	fclose(fileP1);
	fclose(fileP2);
}//end main


/*
 * handler called when aio_read() has stuff to read
 *   First check for any error codes, and if ok, then get the return code
 */
void on_input(int dummy){
	
	//variables 
	int  c;
	int  x;
	char *cp = (char *) kbcbuf.aio_buf;	/* cast to char * */
	

	/* check for errors */
	if ( aio_error(&kbcbuf) != 0 )
		perror("reading failed");
	else 
		/* get number of chars read */
		if ( aio_return(&kbcbuf) == 1 )
		{
		    //gets character from input
		    c = *cp;
		
		    //if flag is high it will allow us to play user to player 2
		    //else you play player one 
		    if(Flag){
			//ending program when Q is pressed
			if ( c == 'Q' || c == EOF )
				done = 1;
			//Changing direction to up when W is pressed
			else if ( c == 'w' ){
			//prevents from going backwards and killing the snake  
			   if(updown1!=1){
				sidetoside1 = 0;
				updown1 = -1;
				head1='^';
			    }
			}
			//Changing direction to down when S is pressed
			else if( c == 's' ){
			    if(updown1!=-1){
				updown1 = 1;
				sidetoside1 = 0;
				head1='v';
			    }	
			}
			//changing direction to left when A is pressed
			else if( c == 'a' ){
			    if(sidetoside1!=1){
				updown1 = 0;
				sidetoside1 = -1;
				head1='<';
			    }
			}
			//changing direction to right when D is pressed 
			else if( c == 'd'){
			    if(sidetoside1!=-1){
				updown1 = 0;
				sidetoside1 = 1;
				head1 = '>';
			    }
			}
		    }//end if(flag)
		    //same as above, but for player one 
		    else{

			if ( c == 'Q' || c == EOF )
				done = 1;

			else if ( c == 'w' ){
			    if(updown!=1){
				sidetoside = 0;
				updown = -1;
				head='^';
			    }
			}
			else if( c == 's' ){
			    if(updown!=-1){
				updown = 1;
				sidetoside = 0;
				head='v';
			    }	
			}
			else if( c == 'a' ){
			    if(sidetoside!=1){
				updown = 0;
				sidetoside = -1;
				head='<';
			    }
			}
			else if( c == 'd'){
			    if(sidetoside!=-1){
				updown = 0;
				sidetoside = 1;
				head = '>';
			    }
			}
		    }
		}
	
	/* place a new request */
	aio_read(&kbcbuf);
}

//this is our interrupt
//this is what moves the players
//and checks for collision 
void on_alarm(int dummy){

	//variables 
	char holder;
	char holder1;
	
	//if flag is high, it will read from player 1 and  print to  player2
	//else, it will read from player 2 and print to player 1
	if(Flag){
	    fseek(fileP1,0,SEEK_SET);
	    fscanf(fileP1,"%3d,%3d,%3d,%3d,%3d",&row,&col,&head,&sidetoside,&updown);

	    fseek(fileP2,0,SEEK_SET);
	    fprintf(fileP2,"%3d,%3d,%3d,%3d,%3d",row1,col1,head1,sidetoside1,updown1);
	
	}
	else{
	    fseek(fileP1,0,SEEK_SET);
	    fprintf(fileP1,"%3d,%3d,%3d,%3d,%3d",row,col,head,sidetoside,updown);

	    fseek(fileP2,0,SEEK_SET);
	    fscanf(fileP2,"%3d,%3d,%3d,%3d,%3d",&row1,&col1,&head1,&sidetoside1,&updown1);
	}
	
	//incramenting score 
	score++;
	score1++;
	
	//check to see if highscore has been borken
	//if so, it will set new highscore
	if(highscore<score){
	    highscore=score;
	}
	//if high score is broken, it will continiously update the score untill player dies
	else if(highscore < score1){
	    highscore=score1;
	}

	signal(SIGALRM, on_alarm);	/* reset, just in case	*/
	mvaddch(row,col, tail);		//prints the tail for player 1 
	

	//for player 1 
	col += sidetoside;		/* move to new column	*/
	row += updown;			//move to new row
	
	//grabs the next possible postion 
	holder = mvinch(row,col);

	//if holder is not a space
	//then collision has occured
	if(holder!=' '){
	    //moving back to delete
	    col -= sidetoside;
	    row -= updown;
	    //clears the players board
	    removeplayer(row, col, tail, head | COLOR_PAIR(1));
	    score=0;			//resets the score
	    row=10;			//resets starting postion
	    col=10;			//resets starting postion 
	    flash();			//flashes the screen 
	}

	//for player 2
	mvaddch(row1,col1, tail1);	//prints the tail 
	col1 += sidetoside1;		/* move to new column	*/
	row1 += updown1;		//move to new row
	
	//looks to see what is at next possible position
	holder1 = mvinch(row1,col1);
	
	//if holder is not a space,
	//then collision has occured.
	if(holder1!=' '){
	    //moving back to delete 
	    col1 -= sidetoside1;
	    row1 -= updown1;
	    //clearing, players board
	    removeplayer(row1, col1, tail1, head1 | COLOR_PAIR(2));
	    score1=0;			//resets the scored
	    row1=15;			//goes to starting positon
	    col1=10;			//goes to starting postion 
	    beep();			//makes sounds 
	}
	//prints the border
	printborder();
	mvaddch( row, col, head| COLOR_PAIR(1) );/* draw new string	*/
	mvaddch( row1, col1, head1| COLOR_PAIR(2) );/* draw new string	*/

	refresh();			/* and show it		*/

	/*
	 * now handle borders
	 */
	//checks for collisions for borders 
	//same basic principle as above
	if ( (sidetoside == -1 && col <= 0)||(updown == -1 && row <= 0) ){
	    col -= sidetoside;
	    row -= updown;
	    removeplayer(row, col, tail, head | COLOR_PAIR(1));
	    score =0;
	    row=10;
	    col=10;
	}	
	else if ( (sidetoside == 1 && col+1 >= COLS) ||
		    (updown == 1 && row+1>=LINES)){
	    col -= sidetoside;
	    row -= updown;
	    removeplayer(row, col, tail, head | COLOR_PAIR(1));
	    score =0;
	    row =10;
	    col =10;
	}

       //opponent***************
	if ( (sidetoside1 == -1 && col1 <= 0)||(updown1 == -1 && row1 <= 0) ){
	    col1 -= sidetoside1;
	    row1 -= updown1;
	    removeplayer(row1, col1, tail1, head1 | COLOR_PAIR(2));
	    score1=0;
	    row1=15;
	    col1=10;
	}	
	else if ( (sidetoside1 == 1 && col1+1 >= COLS) ||
		    (updown1 == 1 && row1+1>=LINES)){
	    col1 -= sidetoside1;
	    row1 -= updown1;
	    removeplayer(row1, col1, tail1, head1 | COLOR_PAIR(2));
	    score1 =0;
	    row1 =15;
	    col1 =10;
	}
}
/*
 * set members of struct.  
 *   First specify args like those for read(fd, buf, num)  and offset
 *   Then  specify what to do (send signal) and what signal (SIGIO)
 */
void setup_aio_buffer()
{
	static char input[1];		      /* 1 char of input */

	/* describe what to read */
	kbcbuf.aio_fildes     = 0;	      /* standard intput */
	kbcbuf.aio_buf        = input;	      /* buffer          */
	kbcbuf.aio_nbytes     = 1;             /* number to read  */
	kbcbuf.aio_offset     = 0;             /* offset in file  */

	/* describe what to do when read is ready */
	kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	kbcbuf.aio_sigevent.sigev_signo  = SIGIO;  /* send sIGIO   */
}
//function removes players from the board 

void removeplayer(int x, int y, int tailID, int headID){

    int i;
    int j;

    //first for loop checks the rows 
    for(j=0;j<79;j++){
	//the second for loop checks for the columns
	for(i=0;i<23;i++){
	    //if something exists there, it will set it as a space
	    //and refresh the screen 
	    if((mvinch(i,j))==tailID || mvinch(i,j)== headID){
		move(i,j);
		addch(' ');
		refresh();
	    }   
	}
    }	
}

//function that prints the boarder 
void printborder(){
    int i;
    //prints the top and bottom horizontal line
    for(i=0;i<80;i++){
	mvaddch(0,i,'*' | COLOR_PAIR(3));
	mvaddch(23,i,'*' | COLOR_PAIR(3));
    }
    //prints the left and right vertical lines
    for(i=0;i<24;i++){
	mvaddch(i,0,'*' | COLOR_PAIR(3));
	mvaddch(i,79,'*' | COLOR_PAIR(3));
    }
    //prints the score for player 1
    attron(COLOR_PAIR(1));  //turns on color
    mvaddstr(23,5,"RED:");
    printw("%04d",score);
    attroff(COLOR_PAIR(1)); //turns off color

    //prints the score for player 2
    attron(COLOR_PAIR(2));  //turns on color
    mvaddstr(23,16,"GREEN:");
    printw("%04d",score1);
    attroff(COLOR_PAIR(2)); //turns off color 
 
    //prints the high score 
    mvaddstr(23,50,"HIGHSCORE:");
    printw("%04d",highscore);

}
