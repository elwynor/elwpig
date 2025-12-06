<pre>

           ____     ____   ______   __
          / __ \   /  _/  / ____/  / /
         / /_/ /   / /   / / __   / / 
        / ____/  _/ /   / /_/ /  /_/  
       /_/      /___/   \____/  (_)   
                                      
                version 1.02
   Copyright (c) 2025 Elwynor Technologies
</pre>

## WHAT IS PIG?

 Pig! is a multi-player dice roll game (that can be played against Mr. Pig,
 the computer player).  The object of the game is simple: to be the first to 
 collect the requisite number of points (set by SysOp) by rolling dice.

## GAMEPLAY
 
 When it is your turn, you roll the dice to accumulate points.  However, if 
 you roll a 7, you immediately lose your turn, and the points that you've 
 accumulated.  If you roll anything else, you accumulate those points.
 
 You may stay at any time while rolling the dice.  If you choose to stay, 
 you will keep the points you have earned - staying "banks" the points,
 so that should you roll a 7 your next turn, you still have the points
 that you had when you stayed.  When you stay, everyone else will get a 
 chance to roll before the dice are returned to you.  
 
 If you reach the set point total first, you win the game!
 
## COMMANDS
 
 Any non-command text that you type will be sent to all players in Pig!
 The following are game commands:
 
 JOIN  ...............  Enters you in the next game
 QUIT  ...............  Removes you from the game
 ROLL  ...............  Roll the dice (Your turn only)
 STAY  ...............  Stay with your points (Your turn only)
 DICE ON/OFF .........  Turns ANSI Dice Display on and off
 HIGH  ...............  Shows score of users.
 SCORE ...............  Shows how many times you have won
 SCAN  ...............  List users in Pig! and their scores
 X     ...............  Exits Pig!
 /userid message .....  Sends a private message (whisper) to userid
 
## INSTALLATION
 
 Simply unzip the Pig! game archive to your BBS server directory,
 add Pig! to your menu, configure the MSG file to your liking, and start
 the BBS!  It's that easy! 

## GAME HISTORY
 
 Pig! was originally developed and sold by Tessier Technologies (TTI).  It
 became part of Galacticomm when TTI was merged into Galacticomm in 1997.
 
 Elwynor Technologies acquired the module in 2005 and ported it to 
 Worldgroup 3.2. 
 
 In 2025, Elwynor Technologies ported it to The Major BBS V10.
 
## LICENSE

 This project is licensed under the AGPL v3. Additional terms apply to 
 contributions and derivative projects. Please see the LICENSE file for 
 more details.

## CONTRIBUTING

 We welcome contributions from the community. By contributing, you agree to the
 terms outlined in the CONTRIBUTING file.

## CREATING A FORK

 If you create an entirely new project based on this work, it must be licensed 
 under the AGPL v3, assign all right, title, and interest, including all 
 copyrights, in and to your fork to Rick Hadsall and Elwynor Technologies, and 
 you must include the additional terms from the LICENSE file in your project's 
 LICENSE file.

## COMPILATION

 This is a Worldgroup 3.2 / Major BBS v10 module. It's compiled using Borland
 C/C++ 5.0 for Worldgroup 3.2. If you have a working Worldgroup 3.2 development
 kit, a simple "make -f ELWPIG" should do it! For Major BBS v10, import this
 project folder in the isv/ subtree of Visual Studio 2022, right click the
 project name and choose build! When ready to build for "release", ensure you
 are building for release.

## PACKING UP

 The DIST folder includes all of the items that should be packaged up in a 
 ELWPIG.ZIP. When unzipped in a Worldgroup 3.2 or Major BBS V10 installation 
 folder, it "installs" the module.
