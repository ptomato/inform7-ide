!----------------------------------------------------------------------------
!   Sound               An Inform 6 library by L. Ross Raszewski to handle 
!                       Inform's sound capabilities. Requires sound card.
!                       CAUTION: MAY BEHAVE UNPREDICTABLY ON SOME INTERPRETERS
!
! Most notably, the "Sequencer" option intermittenly crashes Frotz on my 
! system rather dramatically.  I'm not sure if it's my interpreter or my 
! computer.  At any rate, be careful.  I've had no trouble with the other 
! operations, but that's no guarantee.
!
! This system consumes three global variables and one array:
! play_sounds -  If play_sounds=1, sounds will be played, if 0, they will not.
! sound_vol -  a number from 1 to 8, setting the volume 
! playing -  A bit array holding three bites used internally. ->0 indicates 
!               whether or not PlaySound has been called yet.
!               ->1 indicates the currenly playing sound effect number
!               ->2 is used by internal operations
!
!  This library plays sound effects of the formats described in Stefan 
! Jokisch's treatise on Infocom sound formats.  The actual conventions of the 
! format are handled by the interpreter, but, and there's a lot more on this in
! the Specification of the Z-Machine, sound_effects 1 and 2 are beeps, and 3 
! and up are sound files in an arcane and proprietary format. (There's a beta 
! version of SOX that an write to them, however.) Once you have the files where
! your interpreter wants them (usually the "sound" subdirectory), the 
! @sound_effect opcode plays these sounds.  However, the nature of 
! @sound_effect is such that its use isn't intuative.  Indeed, there's three 
! lines of assembly I had to stick in to make it work (liberally stolen from 
! Sherlock)  This library makes this useable.
!
! Usage:
! To play a sound:
! PlaySound(sound_number);
! Options:
! PlaySound(sound_number,reps);
!               - plays a sound effect a set number of times
! PlaySound(sound_number,Routine,[option]);
!               - Plays a sound effect, and runs Routine when the effect 
!                 has finished.  The third argument is stored in the global 
!                 variable sequence.
! Features:
!       If you're using my footnote library, a footnote will be called 
!       the first time PlaySound is used.  The number of that footnote 
!       should be stored in a constant "SoundNote".  Such a footnote 
!       could say something like "Type "SOUND OFF" to disable sounds"
!
!  Routines included:
!       Sequencer:  Call PlaySound(Sequencer,array);.  This will cause 
!               each sound effect in the array to be played in order.
!               array->0 MUST be the number of sounds to play, however.
!               Note: This only works about one time in five on my system
!               if anyone could explain why, I'd be much obliged.
!       Repeat and Fade: Call PlaySound(sound_number,RepAndFade); to
!               cause a sound effect to repeat and fade out.
!       Fade In: Call PlaySound(sound_number, FadeIn); and the sound effect 
!               will repeat while fading in.
!
!  End-User Functions:
!      Sound    -       Toggles sound on and off
!      Sound On -       Turns sound on
!      Sound Off-       Turns sound off
!      Sound Up -       Turns volume up
!      Sound Down-      Turns volume down
!      Sound (number)-  Sets volume
!
! Debugging Suite:
!       Audio (number)- Plays the corresponding sound effect
!       Audio (2 numbers) Plays the corresponding effect a number of times
!       Audio FIN #  -  Fades in the sound effect
!       Audio RF #   -  Repeats and fades sound effect
!



global play_sounds=1;
global sound_vol=5;
global sequence;
array playing -> 3;
[ PlaySound i Rout k j l;
        if (play_sounds==0) rtrue;
        #IFDEF SoundNote;
        if (playing->0==0) print "^", (Note) SoundNote, "^";
        playing->0=1;
        #ENDIF;
        if (i ofclass Routine) {sequence=Rout; indirect(i); rtrue;};
        if (~~(Rout ofclass Routine)){j=Rout; Rout=0;};
        if (j==0) j=1;
        if (Rout==0) Rout=StopSound;
        l=sound_vol;
        if (Rout==FadeIn) l=1;
        playing->1=i;
        sequence=k;
        @log_shift j 8 sp;
        @or sp l sp;
        @sound_effect i 2 sp Rout;
];
[ StopSound i;
        i=playing->1;
        @sound_effect i 4;
];
[ FadeIn;
        playing->2=1;
        FIner();
];
[ FIner i j;
        playing->2=playing->2+1;
        i=playing->1;
        j=playing->2;
        @log_shift 1 8 sp;
        @or sp j sp;
        if (j<=sound_vol)
                @sound_effect i 2 sp FIner;
];

[ RepAndFade;
        playing->2=sound_vol;
        RnF();              
];
[ RnF j i;
        playing->2=playing->2-1;
        i=playing->1;
        j=playing->2;
        @log_shift 1 8 sp;
        @or sp j sp;
        if (j>=1)
                @sound_effect i 2 sp RnF;
];
[ Sequencer;
        playing->2=0;
        SeqNext();
];
[ SeqNext j i k;
        playing->2=playing->2+1;
        j=playing->2;
        if (j>sequence->0) rtrue;
        i=sequence->j;
        @log_shift 1 8 k;
        @or k sound_vol k;
        @sound_effect i 2 k SeqNext;
];

[ SoundToggleSub;
        if (play_sounds==1) <<SoundsOff>>;
        if (play_sounds==0) <<SoundsOn>>;
];
[ SoundVolSub;
        if (noun>8 || noun <=0) "Please specify a volume level between 1 and 8.";
        sound_vol=noun;
        "Sound volume set to ", sound_vol, ".";
];
[ SoundUpSub;
        if (sound_vol==8) "Maximum volume";
        sound_vol++;
        "Sound volume set to ", sound_vol, ".";
];
[ SoundDownSub;
        if (sound_vol==1) "Minimum volume";
        sound_vol--;
        "Sound volume set to ", sound_vol, ".";
];
[ SoundsOnSub;
        play_sounds=1;
        "Sound on.";
];
[ SoundsOffSub;
        play_sounds=0;
        "Sound off.";
];
verb "sound" "sounds" "volume" *                       -> SoundToggle
             * "on"                                    -> SoundsOn
             * "off"                                   -> SoundsOff
             * "up"                                    -> SoundUp
             * "down"                                  -> SoundDown 
             * number                                  -> SoundVol;

#IfDef DEBUG;
[ AudioSub;
PlaySound(noun);
];
[ AudioRSub;
PlaySound(noun,RepAndFade);
];
[ AudioFINSub;
PlaySound(noun,FadeIn);
];
[ AudioXRSub;
PlaySound(noun,second);
];

verb "audio" * number                                   ->Audio
             * number number                            ->AudioXR
             * "Fin" number                             ->AudioFIN
             * "RF" number                              ->AudioR;             
#ENDIF;
!---------------------------------------------------------------------------
