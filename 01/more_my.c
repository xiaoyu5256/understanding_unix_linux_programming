/*  more_my.c  more
 *	read and print 24 lines then pause for a few special commands
 *      feature of version 0.3: reads from /dev/tty for commands
 *      todos:
 *          'more?'不随内容滚动
 *          [v]'more?'前显示当前内容百分比
 *          输入空格或者q就执行，不需要输入回车执行 
 *          按b回到上一页
 *          [v]根据屏幕行数来显示内容，而不是指定的24行
 */
#include    <stdlib.h>
#include	<stdio.h>
#include    <sys/ioctl.h>
/* #define	PAGELEN	24 */
#define	LINELEN	512

void do_more(FILE *,int);
int see_more(FILE *,int,float);
struct winsize get_screen_winsize();
float cur_percent(FILE *);


int main( int ac , char *av[] )
{
	FILE	*fp;
    int pageline = get_screen_winsize().ws_row -1;
        
	if ( ac == 1 )
		do_more( stdin ,pageline);
	else
		while ( --ac )
			if ( (fp = fopen( *++av , "r" )) != NULL )
			{
				do_more( fp,pageline ) ; 
				fclose( fp );
			}
			else
				exit(1);
	return 0;
}

void do_more( FILE *fp ,int pageline)
/*
 *  read PAGELEN lines, then call see_more() for further instructions
 */
{
	char	line[LINELEN];
	int	num_of_lines = 0;
	int	reply;
    float percent;
	FILE	*fp_tty;

	fp_tty = fopen( "/dev/tty", "r" );	   /* NEW: cmd stream   */
	if ( fp_tty == NULL )			   /* if open fails     */
		exit(1);                           /* no use in running */

	while ( fgets( line, LINELEN, fp ) ){		/* more input	*/
		if ( num_of_lines == pageline ) {	/* full screen?	*/
            percent = cur_percent(fp);
			reply = see_more(fp_tty,26,percent);  /* NEW: pass FILE *  */
			if ( reply == 0 )		/*    n: done   */
				break;
			num_of_lines -= reply;		/* reset count	*/
		}
		if ( fputs( line, stdout )  == EOF )	/* show line	*/
			exit(1);			/* or die	*/
		num_of_lines++;				/* count it	*/
	}
}

int see_more(FILE *cmd,int pageline,float percent)				   /* NEW: accepts arg  */
/*
 *	print message, wait for response, return # of lines to advance
 *	q means no, space means yes, CR means one line
 */
{
	int	c;

	printf("\033[7m current:%.1f%% more? \033[m",percent);		/* reverse on a vt100	*/
    
	while( (c=getc(cmd)) != EOF )		/* NEW: reads from tty  */
	{
		if ( c == 'q' )			/* q -> N		*/
			return 0;
		if ( c == ' ' )			/* ' ' => next page	*/
			return pageline;		/* how many to show	*/
		if ( c == '\n' )		/* Enter key => 1 line	*/
			return 1;		
	}
	return 0;
}
/*
 * 获取屏幕的行数和列数
 */
struct winsize get_screen_winsize(){
    struct winsize w;
    ioctl(0,TIOCGWINSZ,&w);
    return w;
}
/*
 * 获取文件当前位置的百分比
 */
float cur_percent(FILE *fp){
    int cur_position = ftell(fp);
    int total;
    fseek(fp,0L,SEEK_END);
    total = ftell(fp);
    fseek(fp,cur_position,SEEK_SET);
    return 100.0*cur_position/total;
}
