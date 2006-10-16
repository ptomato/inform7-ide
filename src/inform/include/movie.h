!------------------------------------------------------- -*- Inform -*- ----
!  Movie.H              An Inform 6.0 library to put non-interactive 
!   Version 2           "cut scenes" into a game.  By L. Ross Raszewski
!
! This is a minor re-release as I just realized I uploaded the wrong copy
! to gmd.  Sorry.
!
!
! I know what you're thinking.  You're thinking, "non-interactive cut scenes"?
! What would I want a thing like that for?  Well, It may at times be useful,
! or at least novel, to have the player suddenly be stopped and watch a sort 
! of anecdote.  It could be used for an introduction, and endgame, or a 
! flashback.  The player may, if they wish, choose not to view movies 
! automatically.  This library allows players to review movies they have 
! already seen.
! Usage:
! In a print statement: (PlayMovie) 1
! (Call PlayMovie(movie,1); if you want the movie to play regardless of 
! autoplay and replay settings!)
! The programmer must provide a DoMovie(movie) sub, which reads its argument, 
! and writes the movie based on it as such:
! [ DoMenu m;
!       switch(m){1: "This is the first movie";
!                  ... };     
! ];
!
! The first time a call to PlayMovie(movie); is made, if you have a footnote
! library installed (especially mine) define the constant MovieNote with a 
! value equal to one of your footnotes, and that footnote will be called.
! (To alert players to the fact that they can watch movies later).
! If the global "ShowMovies" is equal to 1, movies will not be shown, but 
! a message will alert players to the fact that one is being played.  
! If the global replayMovie is set to 1, the movie will only play the first
! time it is callled, and the alert message will be shown all subsequent 
! times.  The global nextmovie can be read, but should be left alone by 
! the game (unless you're trying to do something really sneaky).
!
! In addition to DoMovie, the user must provide an array (a bit array 
! will do) called 'movies', of a length _greater_ then the number of movies 
! in the game (because I use movies-->0 for internal things.)  
! The game should provide a word array, MovieTitle-->, which names the movies
! (This MUST start at MovieTitle-->1, not -->0, for internal reasons.)
! and a MovieTL-> which holds (again, starting one entry in) the number of 
! letters for the corresponding MovieTitle _divided by 2_ for DoMenu.
! The user need also define either a global or constant (depending on how 
! you plan to use it) called MMTitle (The banner you want to display when the 
! movies are called from a menu) and another called MMWidth, (half the width 
! of the MMTitle)  
!
! When called from the screen, PlayMovie will prompt for a keypress 
! (the WaitForKey("Message"); routine is available for public use.) 
! then clear the screen, show the movie, wait for a keypress, then clear the 
! screen again.
!
! Typing "Movie" will bring up the movie menu.  The names of viewed movies will
! appear in the same order as the player saw them, and the player can view his 
! choice of movie.  "Movie On" and "Movie Off" will enable and disable movie 
! autoplaying. (A verb to change the replay mode is "left as an exercise to the
! reader")
!
! Calling DoMovie alone will write the movie but not update the counter.  The 
! movie will not appear on on the Movie review screen. (Nor will the screen be 
! blanked)
!
! MoviePlayed(movie) returns true if the movie in question has been played, false 
! otherwise
! FindMovie(i) finds which movie was "i-th" played.
!
!
! Questions, Comments, bugs, e-mail me at rraszews@skipjack.bluecrab.org




global ShowMovies=0;
global nextmovie=1;
global replayMovie;
[ PlayMovie movie flag;
   #IFDEF MovieNote;
   if (movies-->0==0) { Note(MovieNote); 
                        new_line; movies-->0=1;};
   #ENDIF;
   if (flag ~= 1 &&(ShowMovies==1 || (MoviePlayed(movie)==1 && replayMovie==1)))
                        { style underline; print "^^[Movie: "; 
                              print (string) MovieTitle-->movie;
                              print "]^^"; style roman; 
                if (MoviePlayed(movie)==0) {
                        movies-->nextmovie=movie;
                        nextmovie++;};
                              rtrue;};
        WaitForKey("^^^Press [SPACE]");
        @erase_window -1;
   
   DoMovie(movie);
   WaitForKey("^^^Press [SPACE]"); @erase_window -1;
                if (MoviePlayed(movie)==0) {
                        movies-->nextmovie=movie;
                        nextmovie++;};
print "^^^";
];
[ MoviePlayed movie i;
        for(i=1:i<=nextmovie:i++) {
                if (movies-->i==movie) rtrue;};
        rfalse;
];
[ MovieMenuSub;
        DoMenu(WriteMovieList,MovieRC,MoviePF);
];
[ WriteMovieList i j;
        new_line;
        for(i=1:i<=nextmovie:i++) {
                j=movies-->i;
                if (j~=0) { new_line;
                        spaces(5); print (string) MovieTitle-->j;};
                        };
];
[ FindMovie i;
   return movies-->i;
];
[ MovieRC;
        if (menu_item==0) { item_name=MMtitle; item_width=MMwidth; return (nextmovie-1);};
        item_name=MovieTitle-->FindMovie(menu_item);
        item_width=MovieTL->FindMovie(menu_item);
];
[ MoviePF i;
        i=FindMovie(menu_item);
        DoMovie(i);
];
[ TurnOffMSub;
  ShowMovies=1;
  "Movie autoplay disabled.";
];
[ TurnOnMSub;
  ShowMovies=0;
  "Movie autoplay enabled.";
];

verb meta "Movies" "Movie" *            -> MovieMenu
                        * "off"         -> TurnOffM
                        * "on"          -> TurnOnM;

#IFNDEF WaitForKey;
[ WaitForKey str i;
if (str==0) str="(Please Press Any Key)";
print (string) str;
@read_char 1 0 0 i;
];
#EndIf;
