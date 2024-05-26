#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <Windows.h>
#include <fstream>

using namespace sf; //Using namespace of SFML instead of std (standard)

//Global Variables

//Constants
const int MyTiles = 100; //Max number of tiles allowed
const int TimePerMove = 10; //Maximum allowed time per move in seconds
const int HintsLimit = 5; //Max number of hints allowed
const int Resolution_x = 1920, Resolution_y = 1080; //Resolution of game

//Numeric Values
float Tilesize; //Size of each candy tile in pixels
int board[MyTiles][MyTiles]; //2D Array which acts as the board
int Tiles; //Current number of tiles depending on the user selected level
int GameState = 0; //Defines the state of the game (Title Screen, Level Selector, Actual Game)
int Moves; //Maximum moves allowed per level
int Tiffi_State; //Sets the side character to a random of 4 available characters
int Score; //Current score (number of candies destroyed)
int Target; //Target score to pass
int Hints_Used; //Number of hints currently used
struct cord {
    int Xcord, Ycord;
} Offset; //Offset of the tiles from the top left in pixels
int Highlight; //Checks if current Tile needs to be highlighted
int Current_Tile_I, Current_Tile_J; //Current x,y tile values of mouse
int Last_Tile_I, Last_Tile_J; //Previous x,y tile values of mouse

//Flags
bool Enable_Diagonals = false; //Enable / Disable Diagonal Moves
bool Enable_Elbows = false; //Enable / Disable Elbow Moves
bool IsMouseDown = false; //Checks if mouse is held down
bool Enable_Audio = true; //Enable / Disable Audio
bool IsTimeOut; //Checks if Timer ran out
bool ShouldResetTimer; //Checks if we want to Reset the Timer
bool ShouldUpdateTimer; //Checks if we want to resume / pause the Timer
bool ShouldGiveHints; //Enable / Disable Hints
bool IsUserSwap = false; //See if user swapped Candy (for special candies)

//Graphics
RenderWindow window(VideoMode(Resolution_x, Resolution_y),"Candeez Crush",Style::Fullscreen); //My window
Texture BackgroundTexture, CandiesTexture, BoardTexture, TiffiTexture, RectTexture, CursorTexture, TitleTexture, PlayTexture, LevelsTexture, HintTexture, ExitTexture, OptionsTexture, ScoreBoardTexture, NumsTexture, FailureTexture, MuteTexture, TimerTexture, PassTexture, FailureTimeTexture, ProgressBarTexture; //Textures from Resources Folder
Sprite Background, Candy, Board, Tiffi, MyRectangle, MyCursor, Title, Play, Levels, Hint, Exit, Options, ScoreBoard, Nums, Failure, Mute, Timer, Pass, ProgressBar; //Sprites to display on screen with textures

//Sounds
Music MyMusic; //For Background Song
SoundBuffer Candyfall1, Candyfall2, Candyfall3, Candyfall4, ButtonPress, GoodSwap, Bad_Swap, Spawn, Level_Completed, Level_Started, Level_Failed, Timeout_Warning, Tasty, Sweet, SugarCrush, Frogtastic, Divine, Delicious, shuffle, Special; //Sound Effects from Resources Folder
Sound SoundEffect, SoundEffect2, SoundEffect3, SoundEffect4, SoundEffect5; //Sounds to run on screen with textures

//Timer
Clock MyClock; //Keeps track of time
Time TimeElapsed; //Time elapsed since last reset

//Events
Event e; //Events to Poll

//Function Prototypes
void InitializeGame(); 
void InitializeBoard();
bool IsMouseWithinBoard();
void DrawBoard();
void CalculateIJ();
void SwapTile();
void HighlightCandy();
void AnimationSwap(int, int, int, int);
bool IsMatchAvailable(int&, int&, int&, int&, int&);
void MatchCandies();
bool AnimationDrop(int, int, int, int);
bool IsMoveAvailable();
void IsMoveAvailableLogic(int&, int&, int&, int&);
void Shuffle();
void DrawCursor();
bool TitleScreen();
void DrawComponents();
bool LevelSelector();
void GiveHint();
bool ExitStage();
void SetOptions();
void PlayCandyDropSound();
void PlayButtonPressSound();
void PlaySwapSound(bool);
void PlaySpawnSound();
bool CheckFail();
void DrawMoves();
bool ShowFailure();
void SetMute();
void DrawScore();
void UpdateScore(int, int, int, int);
bool CheckWin();
bool ShowPass();
void ShowTime();
void SaveToFile();
void LoadFile();
void BackgroundMusic();
void PlayRandomSound();
void PlayLevelPass();
void PlayLevelFail();
void PlayLevelStart();
void DrawProgressBar();
void DrawHintNum();
void PlayTimeOutSound(int);
bool PopSpecialCandies();

//Program Code
void main()
{
    InitializeGame();
    BackgroundMusic();

    while (1) //Infinite Loop for GameState switching
    {
        if (GameState == 0) //Title
        {
            while (window.isOpen())
            {
                while (window.pollEvent(e))
                {
                    if (e.type == Event::Closed)window.close();
                    if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (!TitleScreen()) break; //Play button pressed
                if (GameState != 0)break; //Load button pressed
            }
            GameState++;
        }
        if (GameState == 1) //Level Selector
        {
            while (window.isOpen())
            {
                while (window.pollEvent(e))
                {
                    if (e.type == Event::Closed)window.close();
                    if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (!LevelSelector()) break;
                if (GameState != 1)break;
            }
            GameState++;
        }
        if (GameState == 2) //Game Loop
        {
            MyClock.restart(); //Restart clock
            while (window.isOpen())
            {
                while (window.pollEvent(e))
                {
                    if (e.type == Event::Closed)window.close();
                    if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }

                //Draw
                window.clear();
                DrawBoard();
                window.display();

                //Exit
                if (ExitStage()) break;

                //Mute
                SetMute();

                //Mouse
                CalculateIJ();

                //Shuffle
                Shuffle();

                //Hint
                GiveHint();

                //Swap Tiles
                SwapTile();

                //Check Matches
                MatchCandies();

                //Check Win
                CheckWin();
                if (GameState != 2)break;

                //Check Lose
                CheckFail();
                if (GameState != 2)break;
            }
            GameState++;
        }
    }
}

void InitializeGame()
{
    //Setting up Variables and Stuff
    srand(time(0)); //Seeding random values
    Score = Hints_Used = 0;
    TimeElapsed = seconds(0);
    IsTimeOut = ShouldResetTimer = ShouldUpdateTimer = IsUserSwap = false;
    ShouldGiveHints = true;
    Tilesize = 1000.0 / Tiles;
    Offset.Xcord = (Resolution_x - 1000) / 2;
    Offset.Ycord = (Resolution_y - 1000) / 2;
    window.setMouseCursorVisible(false); //Sets Default Mouse to invisible
    window.setFramerateLimit(30); //Limits frames to 30 FPS

    //Loading Textures
    BackgroundTexture.loadFromFile("Resources/bg.png");
    CandiesTexture.loadFromFile("Resources/Candies.png");
    BoardTexture.loadFromFile("Resources/Template.png");
    TiffiTexture.loadFromFile("Resources/Tiffi.png");
    RectTexture.loadFromFile("Resources/Rect.png");
    CursorTexture.loadFromFile("Resources/Cursor.png");
    TitleTexture.loadFromFile("Resources/Title.jpg");
    PlayTexture.loadFromFile("Resources/Play.png");
    LevelsTexture.loadFromFile("Resources/Levels.png");
    HintTexture.loadFromFile("Resources/Hint.png");
    ExitTexture.loadFromFile("Resources/Exit.png");
    OptionsTexture.loadFromFile("Resources/Options.png");
    ScoreBoardTexture.loadFromFile("Resources/ScoreBoard.png");
    NumsTexture.loadFromFile("Resources/Nums.png");
    FailureTexture.loadFromFile("Resources/Failure.png");
    MuteTexture.loadFromFile("Resources/Mute.png");
    TimerTexture.loadFromFile("Resources/Timer.png");
    PassTexture.loadFromFile("Resources/Passed.png");
    FailureTimeTexture.loadFromFile("Resources/FailureTime.png");
    ProgressBarTexture.loadFromFile("Resources/bar.png");

    //Smoothing Textures
    ProgressBarTexture.setSmooth(true);
    FailureTimeTexture.setSmooth(true);
    PassTexture.setSmooth(true);
    TimerTexture.setSmooth(true);
    MuteTexture.setSmooth(true);
    FailureTexture.setSmooth(true);
    NumsTexture.setSmooth(true);
    ScoreBoardTexture.setSmooth(true);
    OptionsTexture.setSmooth(true);
    ExitTexture.setSmooth(true);
    HintTexture.setSmooth(true);
    LevelsTexture.setSmooth(true);
    PlayTexture.setSmooth(true);
    TitleTexture.setSmooth(true);
    CursorTexture.setSmooth(true);
    RectTexture.setSmooth(true);
    TiffiTexture.setSmooth(true);
    BoardTexture.setSmooth(true);
    CandiesTexture.setSmooth(true);

    //Setting up Sprites
    Background.setTexture(BackgroundTexture);

    Candy.setTexture(CandiesTexture);

    Board.setTexture(BoardTexture);
    Board.setColor(Color(255, 255, 255, 128)); //Setting Alpha (trnasparency) to 50%
    Board.setPosition(Offset.Xcord, Offset.Ycord);

    MyRectangle.setTexture(RectTexture);
    MyRectangle.setScale(Tilesize / 200.0, Tilesize / 200.0); //Adjusting my highlight box to the tile size

    MyCursor.setTexture(CursorTexture);

    Tiffi.setTexture(TiffiTexture);
    Tiffi_State = rand() % 4; //Choose between characters randomly
    Tiffi.setTextureRect(IntRect(Tiffi_State * 690, 0, 345, 393)); //first two are x,y coords from file, second two are x,y dimension of each character to be picked
    Tiffi.setPosition(57.5, 660);

    Title.setTexture(TitleTexture);

    Play.setTexture(PlayTexture);

    Levels.setTexture(LevelsTexture);

    Hint.setTexture(HintTexture);

    Exit.setTexture(ExitTexture);

    Options.setTexture(OptionsTexture);

    ScoreBoard.setTexture(ScoreBoardTexture);
    ScoreBoard.setPosition((Offset.Xcord - 400) / 2.0 + 1000 + Offset.Xcord, Offset.Ycord + 110);

    Nums.setTexture(NumsTexture);
    Nums.setScale(1.5, 1.5);

    Mute.setTexture(MuteTexture);
    if (Enable_Audio) Mute.setTextureRect(IntRect(0, 0, 500, 500));
    else Mute.setTextureRect(IntRect(500, 0, 500, 500));

    Timer.setTexture(TimerTexture);
    Timer.setPosition(30, 130);

    Pass.setTexture(PassTexture);

    ProgressBar.setTexture(ProgressBarTexture);
    ProgressBar.setPosition((Offset.Xcord - 400) / 2.0 + 1000 + Offset.Xcord, Offset.Ycord + 110);

    //Loading Audio
    Candyfall1.loadFromFile("Resources/CandyFall1.ogg");
    Candyfall2.loadFromFile("Resources/CandyFall2.ogg");
    Candyfall3.loadFromFile("Resources/CandyFall3.ogg");
    Candyfall4.loadFromFile("Resources/CandyFall4.ogg");
    ButtonPress.loadFromFile("Resources/Button.ogg");
    GoodSwap.loadFromFile("Resources/GoodSwap.ogg");
    Bad_Swap.loadFromFile("Resources/BadSwap.ogg");
    Spawn.loadFromFile("Resources/Spawn.ogg");
    Level_Completed.loadFromFile("Resources/Level_completed.ogg");
    Level_Started.loadFromFile("Resources/Level_start.ogg");
    Level_Failed.loadFromFile("Resources/Level_failed.ogg");
    Timeout_Warning.loadFromFile("Resources/Time_warning.ogg");
    Tasty.loadFromFile("Resources/Tasty.ogg");
    Sweet.loadFromFile("Resources/Sweet.ogg");
    SugarCrush.loadFromFile("Resources/Sugar_crush.ogg");
    Frogtastic.loadFromFile("Resources/Frogtastic.ogg");
    Divine.loadFromFile("Resources/Divine.ogg");
    Delicious.loadFromFile("Resources/Delicious.ogg");
    shuffle.loadFromFile("Resources/shuffle.ogg");
    Special.loadFromFile("Resources/Special.ogg");

    //Setting up Sound
    SoundEffect3.setBuffer(Timeout_Warning);
    SoundEffect3.stop(); //to stop tick tick when gamestate changes

    //Board
    InitializeBoard();
}

void InitializeBoard()
{
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            board[i][j] = rand() % 6; //Random candies between 0-5
        }
    }
}

bool IsMouseWithinBoard()
{
    if ((Mouse::getPosition(window).x >= Offset.Xcord) && (Mouse::getPosition(window).y >= Offset.Ycord) && (Mouse::getPosition(window).x <= Offset.Xcord + (Tiles * Tilesize)) && (Mouse::getPosition(window).y <= Offset.Ycord + (Tiles * Tilesize)))
    {
        return true;
    }
    return false;
}

void DrawBoard()
{
    DrawComponents();

    //Draw Candies
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (Current_Tile_I == i && Current_Tile_J == j && IsMouseWithinBoard())
            {
                Highlight = 760; //Source Picture Y-coordinate for highlighted candy
                Candy.setPosition((i * Tilesize) + Offset.Xcord - (0.025 * Tilesize), (j * Tilesize) + Offset.Ycord - (0.025 * Tilesize)); //Centered Position if highlighted
                Candy.setScale(Tilesize / 800.0 * 1.05, Tilesize / 800.0 * 1.05); //Increase Size by 5% if highlighted
            }
            else
            {
                Highlight = 0;
                Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                Candy.setScale(Tilesize / 800.0, Tilesize / 800.0);
            }
            Candy.setTextureRect(IntRect(board[i][j] * 760, Highlight, 760, 760)); //Set Dynamic Texture using Board[i][j]'s value
            window.draw(Candy);
        }
    }

    //Draw Cursor
    DrawCursor();
}

void CalculateIJ()
{
    Current_Tile_I = (Mouse::getPosition(window).x - Offset.Xcord) / Tilesize; //Relative to game window
    Current_Tile_J = (Mouse::getPosition(window).y - Offset.Ycord) / Tilesize;
}

void SwapTile()
{
    static bool IsSwapping = false; //To allow only one swap per click

    Last_Tile_I = Current_Tile_I; //Updating last and current tiles
    Last_Tile_J = Current_Tile_J;

    int temp; //Placeholder variable for use where needed

    while (Mouse::isButtonPressed(Mouse::Left) && !IsSwapping) //Loop while the user is pressing left click and there's no previous swap going on
    {
        if (!IsMouseWithinBoard()) //If Mouse outside board automatically dont swap anything
        {
            IsSwapping = false;
            return;
        }

        HighlightCandy(); //Box highlight
        CalculateIJ(); //Keep calculating new current and previous tiles while mouse is down so we can determine when it changes

        //If there's any mouse movement as the current and previous tiles changed
        if ((Current_Tile_I < Last_Tile_I && Current_Tile_J == Last_Tile_J) || //Left Movement
            (Current_Tile_I > Last_Tile_I && Current_Tile_J == Last_Tile_J && Current_Tile_I < Tiles) || //Right Movement
            (Current_Tile_I == Last_Tile_I && Current_Tile_J < Last_Tile_J) || //Up Movement
            (Current_Tile_I == Last_Tile_I && Current_Tile_J > Last_Tile_J && Current_Tile_J < Tiles)) //Down Movement
        {
            int x1, y1, x2, y2; //Coords of the two tiles that got swapped

            if (Current_Tile_I < Last_Tile_I && Current_Tile_J == Last_Tile_J) //Left Swap
            {
                x1 = Last_Tile_I;
                y1 = y2 = Last_Tile_J;
                x2 = Last_Tile_I - 1;
            }
            else if (Current_Tile_I > Last_Tile_I && Current_Tile_J == Last_Tile_J && Current_Tile_I < Tiles) //Right Swap
            {
                x1 = Last_Tile_I;
                y1 = y2 = Last_Tile_J;
                x2 = Last_Tile_I + 1;
            }
            else if (Current_Tile_I == Last_Tile_I && Current_Tile_J < Last_Tile_J) //Top Swap
            {
                x1 = x2 = Last_Tile_I;
                y1 = Last_Tile_J;
                y2 = Last_Tile_J - 1;
            }
            else //Bottom Swap
            {
                x1 = x2 = Last_Tile_I;
                y1 = Last_Tile_J;
                y2 = Last_Tile_J + 1;
            }

            AnimationSwap(x1, y1, x2, y2); //Animate the swap
            std::swap(board[x1][y1], board[x2][y2]); //Swap board values

            if (board[x1][y1] >= 6 || board[x2][y2] >= 6) //Check if special candy swapped
            {
                if (Enable_Audio) PlaySwapSound(true); //Valid swap
                IsUserSwap = true;
                PopSpecialCandies(); //Pop pop
                Moves--; //Reduce moves
                ShouldResetTimer = true; //Reset timer
                ShouldUpdateTimer = false; //Stop timer until all matches made and animations end
                SoundEffect3.pause(); //Stop timeout sound
            }
            else
            {
                if (!IsMatchAvailable(temp, temp, temp, temp, temp)) //Reverse swap if not a valid move
                {
                    if (Enable_Audio) PlaySwapSound(false);
                    AnimationSwap(x1, y1, x2, y2);
                    std::swap(board[x1][y1], board[x2][y2]); //Swap back to original board
                }
                else //Legit Swap
                {
                    if (Enable_Audio) PlaySwapSound(true);
                    Moves--; //Reduce moves
                    ShouldResetTimer = IsUserSwap = true;
                    ShouldUpdateTimer = false;
                    SoundEffect3.pause();
                }
            }
            IsSwapping = true;
            return;
        }
    }
    if (!Mouse::isButtonPressed(Mouse::Left)) IsSwapping = false;
}

void HighlightCandy()
{
    window.clear();
    DrawBoard();
    MyRectangle.setPosition((Current_Tile_I * Tilesize) + Offset.Xcord, (Current_Tile_J * Tilesize) + Offset.Ycord);
    window.draw(MyRectangle);
    //Draw Cursor
    DrawCursor();
    window.display();
}

void AnimationSwap(int x1, int y1, int x2, int y2)
{
    double speed = 5;
    for (double k = 0; k <= Tilesize; k += Tilesize / speed)
    {
        window.clear();
        DrawComponents();

        //Draw Candies
        for (int i = 0; i < Tiles; i++)
        {
            for (int j = 0; j < Tiles; j++)
            {
                if (i == x1 && j == y1) //If starting candy
                {
                    if (x2 > x1 && y1 == y2) Candy.setPosition((i * Tilesize) + Offset.Xcord + k, (j * Tilesize) + Offset.Ycord); //Go right
                    else if (x2 < x1 && y1 == y2) Candy.setPosition((i * Tilesize) + Offset.Xcord - k, (j * Tilesize) + Offset.Ycord); //Go left
                    else if (x2 == x1 && y1 > y2) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord - k); //Go up
                    else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k); //Go down
                }
                else if (i == x2 && j == y2) //If ending candy
                {
                    if (x2 > x1 && y1 == y2) Candy.setPosition((i * Tilesize) + Offset.Xcord - k, (j * Tilesize) + Offset.Ycord); //Go left
                    else if (x2 < x1 && y1 == y2) Candy.setPosition((i * Tilesize) + Offset.Xcord + k, (j * Tilesize) + Offset.Ycord); //Go right
                    else if (x2 == x1 && y1 > y2) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k); //Go down
                    else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord - k); //Go up
                }
                else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord); //Normal Candy
                Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                window.draw(Candy);
            }
        }
        //Draw Cursor
        DrawCursor();
        window.display();
        Sleep(2);
    }
}

bool IsMatchAvailable(int& x1, int& y1, int& x2, int& y2, int& priority)
{
    //5 in a row

    //Horizontal
    for (int i = 0; i <= Tiles - 5; i++) //Restricted
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j] && board[i][j] == board[i + 4][j])
            {
                x1 = i; y1 = j; x2 = i + 4; y2 = j; priority = 3;
                return true;
            }
        }
    }
    //Vertical
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j <= Tiles - 5; j++)
        {
            if (board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3] && board[i][j] == board[i][j + 4])
            {
                x1 = i; y1 = j; x2 = i; y2 = j + 4; priority = 3;
                return true;
            }
        }
    }
    if (Enable_Diagonals)
    {
        //Diagonal
        //Right Diagonal
        for (int i = 0; i <= Tiles - 5; i++)
        {
            for (int j = 0; j <= Tiles - 5; j++)
            {
                if (board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3] && board[i][j] == board[i + 4][j + 4])
                {
                    x1 = i; y1 = j; x2 = i + 4; y2 = j + 4; priority = 3;
                    return true;
                }
            }
        }
        //Left Diagonal
        for (int i = Tiles - 1; i >= 4; i--)
        {
            for (int j = 0; j <= Tiles - 5; j++)
            {
                if (board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2] && board[i][j] == board[i - 3][j + 3] && board[i][j] == board[i - 4][j + 4])
                {
                    x1 = i; y1 = j; x2 = i - 4; y2 = j + 4; priority = 3;
                    return true;
                }
            }
        }
    }
    //4 in a row

    //Horizontal
    for (int i = 0; i <= Tiles - 4; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j] && board[i][j] == board[i + 3][j])
            {
                x1 = i; y1 = j; x2 = i + 3; y2 = j; priority = 2;
                return true;
            }
        }
    }
    //Vertical
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j <= Tiles - 4; j++)
        {
            if (board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2] && board[i][j] == board[i][j + 3])
            {
                x1 = i; y1 = j; x2 = i; y2 = j + 3; priority = 2;
                return true;
            }
        }
    }
    if (Enable_Diagonals)
    {
        //Diagonal
        //Right Diagonal
        for (int i = 0; i <= Tiles - 4; i++)
        {
            for (int j = 0; j <= Tiles - 4; j++)
            {
                if (board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2] && board[i][j] == board[i + 3][j + 3])
                {
                    x1 = i; y1 = j; x2 = i + 3; y2 = j + 3; priority = 2;
                    return true;
                }
            }
        }
        //Left Diagonal
        for (int i = Tiles - 1; i >= 3; i--)
        {
            for (int j = 0; j <= Tiles - 4; j++)
            {
                if (board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2] && board[i][j] == board[i - 3][j + 3])
                {
                    x1 = i; y1 = j; x2 = i - 3; y2 = j + 3; priority = 2;
                    return true;
                }
            }
        }
    }
    //3 in a row

    //Horizontal
    for (int i = 0; i <= Tiles - 3; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (board[i][j] == board[i + 1][j] && board[i][j] == board[i + 2][j])
            {
                x1 = i; y1 = j; x2 = i + 2; y2 = j; priority = 1;
                return true;
            }
        }
    }
    //Vertical
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j <= Tiles - 3; j++)
        {
            if (board[i][j] == board[i][j + 1] && board[i][j] == board[i][j + 2])
            {
                x1 = i; y1 = j; x2 = i; y2 = j + 2; priority = 1;
                return true;
            }
        }
    }
    if (Enable_Diagonals)
    {
        //Diagonal
        //Right Diagonal
        for (int i = 0; i <= Tiles - 3; i++)
        {
            for (int j = 0; j <= Tiles - 3; j++)
            {
                if (board[i][j] == board[i + 1][j + 1] && board[i][j] == board[i + 2][j + 2])
                {
                    x1 = i; y1 = j; x2 = i + 2; y2 = j + 2; priority = 1;
                    return true;
                }
            }
        }
        //Left Diagonal
        for (int i = Tiles - 1; i >= 2; i--)
        {
            for (int j = 0; j <= Tiles - 3; j++)
            {
                if (board[i][j] == board[i - 1][j + 1] && board[i][j] == board[i - 2][j + 2])
                {
                    x1 = i; y1 = j; x2 = i - 2; y2 = j + 2; priority = 1;
                    return true;
                }
            }
        }
    }
    if (Enable_Elbows)
    {
        //Elbow
        //TopLeft Elbow
        for (int i = 0; i < Tiles - 1; i++)
        {
            for (int j = 0; j < Tiles - 1; j++)
            {
                if (board[i][j] == board[i + 1][j] && board[i][j] == board[i][j + 1])
                {
                    x1 = i; y1 = j; x2 = i + 1; y2 = j + 1; priority = 1;
                    return true;
                }
            }
        }
        //TopRight Elbow
        for (int i = 1; i < Tiles; i++)
        {
            for (int j = 0; j < Tiles - 1; j++)
            {
                if (board[i][j] == board[i - 1][j] && board[i][j] == board[i][j + 1])
                {
                    x1 = i; y1 = j; x2 = i - 1; y2 = j + 1; priority = 1;
                    return true;
                }
            }
        }
        //BottomLeft Elbow
        for (int i = 0; i < Tiles - 1; i++)
        {
            for (int j = 1; j < Tiles; j++)
            {
                if (board[i][j] == board[i + 1][j] && board[i][j] == board[i][j - 1])
                {
                    x1 = i; y1 = j; x2 = i + 1; y2 = j - 1; priority = 1;
                    return true;
                }
            }
        }
        //BottomRight Elbow
        for (int i = 1; i < Tiles; i++)
        {
            for (int j = 1; j < Tiles; j++)
            {
                if (board[i][j] == board[i - 1][j] && board[i][j] == board[i][j - 1])
                {
                    x1 = i; y1 = j; x2 = i - 1; y2 = j - 1; priority = 1;
                    return true;
                }
            }
        }
    }
    //No Match
    return false;
}

void MatchCandies()
{
    int matchcount = 0; //Number of matches made
    int x1, x2, y1, y2, temp; //Starting/ending coordinate of matched candeez
    while (IsMatchAvailable(x1, y1, x2, y2, temp))
    {
        if (!AnimationDrop(x1, y1, x2, y2)) return;
        UpdateScore(x1, y1, x2, y2);
        matchcount++;
    }
    if (matchcount >= 3 && Enable_Audio) PlayRandomSound(); //Sweet, Fantastic, etc
    if (ShouldResetTimer)
    {
        MyClock.restart();
        ShouldResetTimer = false;
        ShouldUpdateTimer = true;
        SoundEffect3.stop();
    }
}

bool AnimationDrop(int x1, int y1, int x2, int y2)
{
    double speed = 5;
    if (y1 == y2) //Horizontal
    {
        if (x2 - x1 >= 3) //Special Candy
        {
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(i >= x1 && i <= x2 && j == y1)) //Only draw non matched candies, dont draw popped candies
                        {
                            if ((i >= x1 && i <= x2 && j < y1 && IsUserSwap && Current_Tile_I != i) || (i >= x1 && i <= x2 && j < y1 && !IsUserSwap && i != x1)) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                        else if ((i == Current_Tile_I && j == y1 && IsUserSwap) || (i == x1 && j == y1 && !IsUserSwap))
                        {
                            Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect((3 + x2 - x1) * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
            if (y1 - 1 > 0) if (Enable_Audio) PlayCandyDropSound();
            for (int i = x1; i <= x2; i++) for (int j = y1 - 1; j >= 0; j--) if ((i != Current_Tile_I && IsUserSwap) || (i != x1 && !IsUserSwap)) board[i][j + 1] = board[i][j];
            for (int i = x1; i <= x2; i++) if ((i != Current_Tile_I && IsUserSwap) || (i != x1 && !IsUserSwap)) board[i][0] = rand() % 6;
            if (IsUserSwap) board[Current_Tile_I][y1] = 3 + x2 - x1;
            else board[x1][y1] = 3 + x2 - x1;
            speed = 20;
            if (Enable_Audio) PlaySpawnSound();
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();
                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (i >= x1 && i <= x2 && j == 0 && i != Current_Tile_I && IsUserSwap || (i >= x1 && i <= x2 && j == 0 && i != x1 && !IsUserSwap)) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
        }
        else
        {
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(i >= x1 && i <= x2 && j == y1))
                        {
                            if (i >= x1 && i <= x2 && j < y1) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;

            }
            if (y1 > 0) if (Enable_Audio) PlayCandyDropSound();
            for (int i = x1; i <= x2; i++) for (int j = y1 - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
            for (int i = x1; i <= x2; i++) board[i][0] = rand() % 6;
            speed = 20;
            if (Enable_Audio) PlaySpawnSound();
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();
                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (i >= x1 && i <= x2 && j == 0) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
        }
    }
    else if (x1 == x2) //Vertical
    {
        if (y2 - y1 >= 3) //Special Candies
        {
            for (double k = 0; k <= Tilesize * (y2 - y1); k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(j >= y1 && j <= y2 && i == x1))
                        {
                            if (j < y1 && i == x1) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                        else if (i == x1 && j == Current_Tile_J && IsUserSwap)
                        {
                            if (k < Tilesize * (y2 - Current_Tile_J)) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((x1 * Tilesize) + Offset.Xcord, (y2 * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect((3 + y2 - y1) * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                        else if (i == x1 && j == y2 && !IsUserSwap)
                        {
                            Candy.setPosition((x1 * Tilesize) + Offset.Xcord, (y2 * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect((3 + y2 - y1) * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
            if (Enable_Audio) PlayCandyDropSound();
            for (int j = y1 - 1; j >= 0; j--) board[x1][j + y2 - y1] = board[x1][j];
            for (int j = 0; j <= y2 - y1 - 1; j++) board[x1][j] = rand() % 6;
            board[x1][y2] = 3 + (y2 - y1);
            speed = 20;
            if (Enable_Audio) PlaySpawnSound();
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (j <= y2 - y1 - 1 && i == x1) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
        }
        else
        {
            for (double k = 0; k <= Tilesize * (y2 - y1 + 1); k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(j >= y1 && j <= y2 && i == x1))
                        {
                            if (j < y1 && i == x1) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
            if (y1 > 0) if (Enable_Audio) PlayCandyDropSound();
            for (int j = y1 - 1; j >= 0; j--) board[x1][j + y2 - y1 + 1] = board[x1][j];
            for (int j = 0; j <= y2 - y1; j++) board[x1][j] = rand() % 6;
            speed = 20;
            if (Enable_Audio) PlaySpawnSound();
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (j <= y2 - y1 && i == x1) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
        }
    }
    else if (abs(x2 - x1) > 1 && Enable_Diagonals) //Diagonal
    {
        if (x1 < x2) //Right Diagonal
        {
            if (x2 - x1 >= 3) //Special Candies
            {
                int currentI, currentJ;
                if (Current_Tile_I - x1 == Current_Tile_J - y1)
                {
                    currentI = Current_Tile_I;
                    currentJ = Current_Tile_J;
                }
                else
                {
                    currentI = Last_Tile_I;
                    currentJ = Last_Tile_J;
                }
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            if (!(i >= x1 && i <= x2 && j >= y1 && j <= y2 && i - x1 == j - y1))
                            {
                                int column = i - x1;
                                if ((i == x1 + column && j < y1 + column && i >= x1 && i <= x2 && i != currentI && IsUserSwap) || (i == x1 + column && j < y1 + column && i >= x1 && i <= x2 && i != x1 && !IsUserSwap))Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                                else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                            else if ((i == currentI && j == currentJ && IsUserSwap) || (i == x1 && j == y1 && !IsUserSwap))
                            {
                                Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect((3 + x2 - x1) * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (Enable_Audio) PlayCandyDropSound();
                for (int i = x1; i <= x2; i++) if ((i != currentI && IsUserSwap) || (i != x1 && !IsUserSwap)) for (int j = y1 + (i - x1) - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
                for (int i = x1; i <= x2; i++) if ((i != currentI && IsUserSwap) || (i != x1 && !IsUserSwap)) board[i][0] = rand() % 6;
                if (IsUserSwap) board[currentI][currentJ] = 3 + x2 - x1;
                else board[x1][y1] = 3 + x2 - x1;
                speed = 20;
                if (Enable_Audio) PlaySpawnSound();
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            if ((i >= x1 && i <= x2 && j == 0 && i != currentI && IsUserSwap) || (i >= x1 && i <= x2 && j == 0 && i != x1 && !IsUserSwap)) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                            else Candy.setColor(Color(255, 255, 255, 255));
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
            }
            else
            {
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            if (!(i >= x1 && i <= x2 && j >= y1 && j <= y2 && i - x1 == j - y1))
                            {
                                int column = i - x1;
                                if (i == x1 + column && j < y1 + column && i >= x1 && i <= x2)Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                                else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (Enable_Audio) PlayCandyDropSound();
                for (int i = x1; i <= x2; i++) for (int j = y1 + (i - x1) - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
                for (int i = x1; i <= x2; i++) board[i][0] = rand() % 6;
                speed = 20;
                if (Enable_Audio) PlaySpawnSound();
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            if (i >= x1 && i <= x2 && j == 0) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                            else Candy.setColor(Color(255, 255, 255, 255));
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
            }
        }
        else if (x2 < x1) //Left Diagonal
        {
            if (x1 - x2 >= 3) //Special Candies
            {
                int currentI, currentJ;
                if (Current_Tile_I - x2 == y2 - Current_Tile_J)
                {
                    currentI = Current_Tile_I;
                    currentJ = Current_Tile_J;
                }
                else
                {
                    currentI = Last_Tile_I;
                    currentJ = Last_Tile_J;
                }
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            if (!(i <= x1 && i >= x2 && j >= y1 && j <= y2 && i - x2 == y2 - j))
                            {
                                int column = abs(i - x1);
                                if ((i >= x2 && i <= x1 && j < y2 - (i - x2) && i != currentI && IsUserSwap) || (i >= x2 && i <= x1 && j < y2 - (i - x2) && i != x1 && !IsUserSwap))Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                                else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                            else if ((i == currentI && j == currentJ && IsUserSwap) || (i == x1 && j == y1 && !IsUserSwap))
                            {
                                Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect((3 + x1 - x2) * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (Enable_Audio) PlayCandyDropSound();
                for (int i = x2; i <= x1; i++) if ((i != currentI && IsUserSwap) || (i != x1 && !IsUserSwap)) for (int j = y2 - (i - x2) - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
                for (int i = x2; i <= x1; i++) if ((i != currentI && IsUserSwap) || (i != x1 && !IsUserSwap)) board[i][0] = rand() % 6;
                if (IsUserSwap) board[currentI][currentJ] = 3 + x1 - x2;
                else board[x1][y1] = 3 + x1 - x2;
                speed = 20;
                if (Enable_Audio) PlaySpawnSound();
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            if ((i <= x1 && i >= x2 && j == 0 && i != currentI && IsUserSwap) || (i <= x1 && i >= x2 && j == 0 && i != x1 && !IsUserSwap)) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                            else Candy.setColor(Color(255, 255, 255, 255));
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
            }
            else
            {
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            if (!(i <= x1 && i >= x2 && j >= y1 && j <= y2 && i - x2 == y2 - j))
                            {
                                int column = abs(i - x1);
                                if (i >= x2 && i <= x1 && j < y2 - (i - x2))Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                                else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                                Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                                window.draw(Candy);
                            }
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
                if (Enable_Audio) PlayCandyDropSound();
                for (int i = x2; i <= x1; i++) for (int j = y2 - (i - x2) - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
                for (int i = x2; i <= x1; i++) board[i][0] = rand() % 6;
                speed = 20;
                if (Enable_Audio) PlaySpawnSound();
                for (double k = 0; k <= Tilesize; k += Tilesize / speed)
                {
                    window.clear();
                    DrawComponents();

                    //Draw Candies
                    for (int i = 0; i < Tiles; i++)
                    {
                        for (int j = 0; j < Tiles; j++)
                        {
                            Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            if (i <= x1 && i >= x2 && j == 0) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                            else Candy.setColor(Color(255, 255, 255, 255));
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                    //Draw Cursor
                    DrawCursor();
                    window.display();
                    Sleep(2);
                    SetMute();
                    if (ExitStage()) return false;
                    while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
                }
            }
        }
    }
    else if (Enable_Elbows) //Elbow
    {
        for (double k = 0; k <= Tilesize * 2; k += Tilesize / speed)
        {
            window.clear();
            DrawComponents();
            //Draw Candies
            for (int i = 0; i < Tiles; i++)
            {
                for (int j = 0; j < Tiles; j++)
                {
                    if (!((i == x1 && j == y1) || (i == x2 && j == y1) || (i == x1 && j == y2)))
                    {
                        if ((i == x1 && j < y1) || (i == x2 && j < y1 && k <= Tilesize)) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                        else if (i == x2 && j < y1 && k > Tilesize) Candy.setPosition((i * Tilesize) + Offset.Xcord, ((j + 1) * Tilesize) + Offset.Ycord);
                        else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
            }
            //Draw Cursor
            DrawCursor();
            window.display();
            Sleep(2);
            SetMute();
            if (ExitStage()) return false;
            while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
        }
        if (Enable_Audio) PlayCandyDropSound();
        if (y1 < y2) for (int j = y1 - 1; j >= 0; j--) board[x1][j + 2] = board[x1][j]; //TopEnable_Elbows
        else for (int j = y2 - 1; j >= 0; j--) board[x1][j + 2] = board[x1][j]; //BottomEnable_Elbows
        for (int j = y1 - 1; j >= 0; j--)board[x2][j + 1] = board[x2][j];
        for (int j = 0; j <= 1; j++) board[x1][j] = rand() % 6;
        board[x2][0] = rand() % 6;
        speed = 20;
        if (Enable_Audio) PlaySpawnSound();
        for (double k = 0; k <= Tilesize; k += Tilesize / speed)
        {
            window.clear();
            DrawComponents();

            //Draw Candies
            for (int i = 0; i < Tiles; i++)
            {
                for (int j = 0; j < Tiles; j++)
                {
                    Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                    if ((i == x1 && j <= 1) || (i == x2 && j == 0)) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                    else Candy.setColor(Color(255, 255, 255, 255));
                    Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                    window.draw(Candy);
                }
            }
            //Draw Cursor
            DrawCursor();
            window.display();
            Sleep(2);
            SetMute();
            if (ExitStage()) return false;
            while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
        }
    }
    IsUserSwap = false;
    return true;
}

bool IsMoveAvailable()
{
    int x1, y1, x2, y2;
    IsMoveAvailableLogic(x1, y1, x2, y2);
    if (x1 == -1) return false; //No Move
    return true;
}

void IsMoveAvailableLogic(int& x1, int& y1, int& x2, int& y2)
{
    //Special Candies
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (board[i][j] >= 6)
            {
                if (i > 0) { x1 = i; y1 = j; x2 = i - 1; y2 = j; return; }
                else if (i < Tiles - 1) { x1 = i; y1 = j; x2 = i + 1; y2 = j; return; }
                else if (j > 0) { x1 = i; y1 = j; x2 = i; y2 = j - 1; return; }
                else { x1 = i; y1 = j; x2 = i; y2 = j + 1; return; }
            }
        }
    }
    int temp, priority = 1, highestpriority = 0;
    bool found = false;
    for (int i = 0; i < Tiles; i++)
    {
        for (int j = 0; j < Tiles; j++)
        {
            if (j != 0)
            {
                std::swap(board[i][j], board[i][j - 1]);
                if (IsMatchAvailable(temp, temp, temp, temp, priority))
                {
                    found = true;
                    if (priority > highestpriority)
                    {
                        highestpriority = priority;
                        x1 = i;
                        y1 = j;
                        x2 = i;
                        y2 = j - 1;
                    }
                }
                std::swap(board[i][j], board[i][j - 1]);
            }
            if (j != Tiles - 1)
            {
                std::swap(board[i][j], board[i][j + 1]);
                if (IsMatchAvailable(temp, temp, temp, temp, priority))
                {
                    found = true;
                    if (priority > highestpriority)
                    {
                        highestpriority = priority;
                        x1 = i;
                        y1 = j;
                        x2 = i;
                        y2 = j + 1;
                    }
                }
                std::swap(board[i][j], board[i][j + 1]);
            }
            if (i != 0)
            {
                std::swap(board[i][j], board[i - 1][j]);
                if (IsMatchAvailable(temp, temp, temp, temp, priority))
                {
                    found = true;
                    if (priority > highestpriority)
                    {
                        highestpriority = priority;
                        x1 = i;
                        y1 = j;
                        x2 = i - 1;
                        y2 = j;
                    }
                }
                std::swap(board[i][j], board[i - 1][j]);
            }
            if (i != Tiles - 1)
            {
                std::swap(board[i][j], board[i + 1][j]);
                if (IsMatchAvailable(temp, temp, temp, temp, priority))
                {
                    found = true;
                    if (priority > highestpriority)
                    {
                        highestpriority = priority;
                        x1 = i;
                        y1 = j;
                        x2 = i + 1;
                        y2 = j;
                    }
                }
                std::swap(board[i][j], board[i + 1][j]);
            }
        }
    }
    if (!found) x1 = -1; //No Move Available
}

void Shuffle()
{
    bool shuffled = false;
    while (!IsMoveAvailable())
    {
        InitializeBoard();
        shuffled = true;
    }
    if (shuffled && Enable_Audio)
    {
        SoundEffect4.resetBuffer();
        SoundEffect4.setBuffer(shuffle);
        SoundEffect4.play();
    }
}

void DrawCursor()
{
    MyCursor.setPosition(Mouse::getPosition(window).x - 8, Mouse::getPosition(window).y - 5);
    window.draw(MyCursor);
}

bool TitleScreen()
{
    window.clear();

    window.draw(Title);

    //Play Button
    Play.setTextureRect(IntRect(0, 0, 1200, 500));
    if (Mouse::getPosition(window).x >= 840 && Mouse::getPosition(window).x <= 1080 && Mouse::getPosition(window).y >= 540 && Mouse::getPosition(window).y <= 640)
    {
        Play.setPosition(840 - 12, 540 - 5);
        Play.setScale(0.22, 0.22); //10% increase in size
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false) //Unique Click
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            return false; //No more title screen
        }
    }
    else
    {
        Play.setPosition(840, 540);
        Play.setScale(0.2, 0.2);
    }
    window.draw(Play);

    //Load Button
    Play.setTextureRect(IntRect(1200, 0, 1200, 500));
    if (Mouse::getPosition(window).x >= 840 && Mouse::getPosition(window).x <= 1080 && Mouse::getPosition(window).y >= 690 && Mouse::getPosition(window).y <= 790)
    {
        Play.setPosition(840 - 12, 690 - 5);
        Play.setScale(0.22, 0.22);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            LoadFile();
            return true;
        }
    }
    else
    {
        Play.setPosition(840, 690);
        Play.setScale(0.2, 0.2);
    }
    window.draw(Play);

    ExitStage(); //Exit Button Logic
    window.draw(Exit);

    SetMute(); //Mute Button Logic
    window.draw(Mute);

    DrawCursor();

    window.display();
    return true;
}

void DrawComponents()
{
    window.draw(Background);
    window.draw(Board);
    window.draw(ScoreBoard);
    DrawMoves();
    DrawScore();
    DrawProgressBar();
    window.draw(Timer);
    ShowTime();
    window.draw(Tiffi);
    window.draw(Hint);
    DrawHintNum();
    window.draw(Exit);
    window.draw(Mute);
}

bool LevelSelector()
{
    window.clear();

    window.draw(Title);

    //Level Select Buttons
    for (int i = 0; i < 6; i++) //6 buttons in a loop
    {
        Levels.setTextureRect(IntRect(i * 80, 0, 80, 80)); //Loading each button's source texture
        if (Mouse::getPosition(window).x >= 410 + (i * 200) && Mouse::getPosition(window).x <= 410 + (i * 200) + 100 && Mouse::getPosition(window).y >= 540 && Mouse::getPosition(window).y <= 640)
        {
            Levels.setPosition(410 + (i * 200) - 5, 540 - 5);
            Levels.setScale(1.375, 1.375); //10% size increase
            if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
            {
                IsMouseDown = true;
                if (Enable_Audio)
                {
                    PlayButtonPressSound();
                    PlayLevelStart();
                }
                if (Mouse::getPosition(window).x >= 410 + (5 * 200)) Tiles = MyTiles; //100 tiles for special level ??????
                else Tiles = i + 5;
                InitializeGame(); //Reset my tilesize etc
                if (i == 0) // 5x5
                {
                    Background.setTextureRect(IntRect(0, 1080, 1920, 1080));
                    Moves = 15;
                    Target = 80;
                    if (Enable_Diagonals) Target += 75;
                    if (Enable_Elbows) Target += 25;
                }
                else if (i == 1)
                {
                    Background.setTextureRect(IntRect(1920, 0, 1920, 1080));
                    Moves = 15;
                    Target = 80;
                    if (Enable_Diagonals) Target += 180;
                    if (Enable_Elbows) Target += 90;
                }
                else if (i == 2)
                {
                    Background.setTextureRect(IntRect(3840, 1080, 1920, 1080));
                    Moves = 12;
                    Target = 90;
                    if (Enable_Diagonals) Target += 160;
                    if (Enable_Elbows) Target += 100;
                }
                else if (i == 3)
                {
                    Background.setTextureRect(IntRect(1920, 1080, 1920, 1080));
                    Moves = 10;
                    Target = 100;
                    if (Enable_Diagonals) Target += 150;
                    if (Enable_Elbows) Target += 80;
                }
                else if (i == 4)
                {
                    Background.setTextureRect(IntRect(3840, 0, 1920, 1080));
                    Moves = 10;
                    Target = 110;
                    if (Enable_Diagonals) Target += 440;
                    if (Enable_Elbows) Target += 90;
                }
                else //?????
                {
                    Background.setTextureRect(IntRect(0, 0, 1920, 1080));
                    Moves = 99;
                    Target = 10000;
                }
                return false;
            }
        }
        else
        {
            Levels.setPosition(410 + (i * 200), 540);
            Levels.setScale(1.25, 1.25);
        }
        window.draw(Levels);
    }

    ExitStage();
    window.draw(Exit);

    SetMute();
    window.draw(Mute);

    SetOptions();

    DrawCursor();

    window.display();
    return true;
}

void GiveHint()
{
    if (Mouse::getPosition(window).x >= 1800 && Mouse::getPosition(window).x <= 1900 && Mouse::getPosition(window).y >= 20 && Mouse::getPosition(window).y <= 120)
    {
        Hint.setPosition(1800 - 5, 20 - 5);
        Hint.setScale(0.22, 0.22);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (ShouldGiveHints)
            {
                if (Enable_Audio) PlayButtonPressSound();
                int x1, y1, x2, y2; //Coords of two candies to swap
                IsMoveAvailableLogic(x1, y1, x2, y2); //Returns two possible coords of candies to swap
                AnimationSwap(x1, y1, x2, y2); //Play animation between these two candies
                std::swap(board[x1][y1], board[x2][y2]); //Swap temporarily to be same as on screen
                Sleep(200);
                AnimationSwap(x1, y1, x2, y2); //Reverse swap animation
                std::swap(board[x1][y1], board[x2][y2]); //Reverse swap to go back to original board
                Hints_Used++;
                if (Hints_Used == HintsLimit) ShouldGiveHints = false;
            }
            else if (Enable_Audio) PlaySwapSound(false);
        }
    }
    else
    {
        Hint.setPosition(1800, 20);
        Hint.setScale(0.2, 0.2);
    }
}

bool ExitStage()
{
    if (Mouse::getPosition(window).x >= 20 && Mouse::getPosition(window).x <= 120 && Mouse::getPosition(window).y >= 20 && Mouse::getPosition(window).y <= 120)
    {
        Exit.setPosition(20 - 5, 20 - 5);
        Exit.setScale(0.22, 0.22);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            if (GameState == 0) //Title
            {
                window.close();
                exit(0);
            }
            else if (GameState == 1) //Level Selector
            {
                GameState = -1; //-1 because ++ in main makes it 0
                return true;
            }
            else if (GameState == 2)
            {
                SoundEffect3.pause(); //Stop timer sound
                SaveToFile(); //Autosave
                GameState = 0;
                return true;
            }
        }
    }
    else
    {
        Exit.setPosition(20, 20);
        Exit.setScale(0.2, 0.2);
    }
    return false;
}

void SetOptions()
{
    //Enable/Disable Diagonal Options
    if (Mouse::getPosition(window).x >= 585 && Mouse::getPosition(window).x <= 885 && Mouse::getPosition(window).y >= 700 && Mouse::getPosition(window).y <= 850)
    {
        Options.setPosition(585 - 15, 700 - 7.5);
        Options.setScale(1.1, 1.1);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            Enable_Diagonals = !Enable_Diagonals;
        }
    }
    else
    {
        Options.setPosition(585, 700);
        Options.setScale(1, 1);
    }
    if (Enable_Diagonals) Options.setTextureRect(IntRect(0, 0, 300, 150));
    else Options.setTextureRect(IntRect(300, 0, 300, 150));
    window.draw(Options);

    //Enable/Disable Elbow Options
    if (Mouse::getPosition(window).x >= 1035 && Mouse::getPosition(window).x <= 1335 && Mouse::getPosition(window).y >= 700 && Mouse::getPosition(window).y <= 850)
    {
        Options.setPosition(1035 - 15, 700 - 7.5);
        Options.setScale(1.1, 1.1);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            Enable_Elbows = !Enable_Elbows;
        }
    }
    else
    {
        Options.setPosition(1035, 700);
        Options.setScale(1, 1);
    }
    if (Enable_Elbows) Options.setTextureRect(IntRect(900, 0, 300, 150));
    else Options.setTextureRect(IntRect(600, 0, 300, 150));

    window.draw(Options);
}

void PlayCandyDropSound()
{
    int temp = rand() % 4;
    SoundEffect.resetBuffer();
    if (temp == 0) SoundEffect.setBuffer(Candyfall1);
    else if (temp == 1) SoundEffect.setBuffer(Candyfall2);
    else if (temp == 2) SoundEffect.setBuffer(Candyfall3);
    else SoundEffect.setBuffer(Candyfall4);
    SoundEffect.play();
}

void PlayButtonPressSound()
{
    SoundEffect.resetBuffer();
    SoundEffect.setBuffer(ButtonPress);
    SoundEffect.play();
}

void PlaySwapSound(bool good)
{
    SoundEffect.resetBuffer();
    if (good) SoundEffect.setBuffer(GoodSwap);
    else SoundEffect.setBuffer(Bad_Swap);
    SoundEffect.play();
}

void PlaySpawnSound()
{
    SoundEffect2.resetBuffer();
    SoundEffect2.setBuffer(Spawn);
    SoundEffect2.setVolume(50);
    SoundEffect2.play();
}

bool CheckFail()
{
    if (Moves == 0 || IsTimeOut)
    {
        ShouldUpdateTimer = false;
        if (Moves == 0) Failure.setTexture(FailureTexture);
        else Failure.setTexture(FailureTimeTexture);
        Tiffi.setTextureRect(IntRect(Tiffi_State * 690 + 345, 0, 345, 393));
        if (Enable_Audio) PlayLevelFail();
        for (int i = 0; i <= 100; i += 5)
        {
            window.clear();
            DrawBoard();
            Failure.setScale(i / 100.0, i / 100.0);
            Failure.setPosition(Offset.Xcord + (1000 - (i / 100.0 * 1000)) / 2, Offset.Ycord + (1000 - (i / 100.0 * 1000)) / 2);
            window.draw(Failure);
            SetMute();
            window.draw(Mute);
            DrawCursor();
            Sleep(0.1);
            window.display();
        }
        for (int i = 0; i <= 100; i += 5)
        {
            window.clear();
            DrawBoard();
            window.draw(Failure);
            Exit.setScale(i / 100.0 * 0.3, i / 100.0 * 0.3);
            Exit.setPosition(Offset.Xcord + (1000 - (i / 100.0 * 150)) / 2, Offset.Ycord + (1000 - (i / 100.0 * 150)) / 2 + 235);
            window.draw(Exit);
            SetMute();
            window.draw(Mute);
            DrawCursor();
            Sleep(0.1);
            window.display();
        }
        while (ShowFailure());
        std::remove("SaveFile.txt");
        return true;
    }
    return false;
}

void DrawMoves()
{
    if (Moves < 10) //Single Digit
    {
        Nums.setTextureRect(IntRect(Moves * 50, 0, 50, 75));
        Nums.setPosition(1652.5, 147.75);
        window.draw(Nums);
    }
    else //Double Digit
    {
        Nums.setTextureRect(IntRect((Moves / 10) * 50, 0, 50, 75));
        Nums.setPosition(1627.5, 147.75);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Moves % 10) * 50, 0, 50, 75));
        Nums.setPosition(1677.5, 147.75);
        window.draw(Nums);
    }
}

bool ShowFailure()
{
    window.clear();
    DrawBoard();
    window.draw(Failure);
    Exit.setPosition(Offset.Xcord + 425, 700);
    if (Mouse::getPosition(window).x >= Offset.Xcord + 425 && Mouse::getPosition(window).x <= Offset.Xcord + 575 && Mouse::getPosition(window).y >= 700 && Mouse::getPosition(window).y <= 850)
    {
        Exit.setScale(0.33, 0.33);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            Exit.setPosition(20, 20);
            GameState = 0;
            return false;
        }
    }
    else Exit.setScale(0.3, 0.3);
    window.draw(Exit);
    SetMute();
    window.draw(Mute);
    DrawCursor();
    window.display();
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)window.close();
        if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
    }
    return true;
}

void SetMute()
{
    if (Mouse::getPosition(window).x >= 140 && Mouse::getPosition(window).x <= 240 && Mouse::getPosition(window).y >= 20 && Mouse::getPosition(window).y <= 120)
    {
        Mute.setPosition(140 - 5, 20 - 5);
        Mute.setScale(0.22, 0.22);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            PlayButtonPressSound();
            Enable_Audio = !Enable_Audio;
            if (Enable_Audio)
            {
                Mute.setTextureRect(IntRect(0, 0, 500, 500));
                MyMusic.play();
            }
            else
            {
                SoundEffect3.pause();
                Mute.setTextureRect(IntRect(500, 0, 500, 500));
                MyMusic.stop();
            }
        }
    }
    else
    {
        Mute.setPosition(140, 20);
        Mute.setScale(0.2, 0.2);
    }
}

void DrawScore()
{
    if (Score < 10)
    {
        Nums.setTextureRect(IntRect(Score * 50, 0, 50, 75));
        Nums.setPosition(1652.5, 446.25);
        window.draw(Nums);
    }
    else if (Score < 100)
    {
        Nums.setTextureRect(IntRect((Score / 10) * 50, 0, 50, 75));
        Nums.setPosition(1627.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score % 10) * 50, 0, 50, 75));
        Nums.setPosition(1677.5, 446.25);
        window.draw(Nums);
    }
    else if (Score < 1000)
    {
        Nums.setTextureRect(IntRect((Score / 100) * 50, 0, 50, 75));
        Nums.setPosition(1602.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score / 10 % 10) * 50, 0, 50, 75));
        Nums.setPosition(1652.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score % 10) * 50, 0, 50, 75));
        Nums.setPosition(1702.5, 446.25);
        window.draw(Nums);
    }
    else
    {
        Nums.setTextureRect(IntRect((Score / 1000) * 50, 0, 50, 75));
        Nums.setPosition(1577.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score / 100 % 10) * 50, 0, 50, 75));
        Nums.setPosition(1627.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score / 10 % 10) * 50, 0, 50, 75));
        Nums.setPosition(1677.5, 446.25);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((Score % 10) * 50, 0, 50, 75));
        Nums.setPosition(1727.5, 446.25);
        window.draw(Nums);
    }
}

void UpdateScore(int x1, int y1, int x2, int y2)
{
    if (y1 == y2) Score += x2 - x1 + 1; //Horizontal
    else if (x1 == x2) Score += y2 - y1 + 1; //Vertical
    else if (abs(x2 - x1) > 1) Score += abs(x2 - x1) + 1; //Diagonal
    else Score += 3; //Elbow
}

bool CheckWin()
{
    if (Score >= Target)
    {
        ShouldUpdateTimer = false;
        if (Enable_Audio) PlayLevelPass();
        for (int i = 0; i <= 100; i += 5)
        {
            window.clear();
            DrawBoard();
            Pass.setScale(i / 100.0, i / 100.0);
            Pass.setPosition(Offset.Xcord + (1000 - (i / 100.0 * 1000)) / 2, Offset.Ycord + (1000 - (i / 100.0 * 1000)) / 2);
            window.draw(Pass);
            SetMute();
            window.draw(Mute);
            DrawCursor();
            Sleep(0.1);
            window.display();
        }
        for (int i = 0; i <= 100; i += 5)
        {
            window.clear();
            DrawBoard();
            window.draw(Pass);
            Exit.setScale(i / 100.0 * 0.3, i / 100.0 * 0.3);
            Exit.setPosition(Offset.Xcord + (1000 - (i / 100.0 * 150)) / 2, Offset.Ycord + (1000 - (i / 100.0 * 150)) / 2 + 235);
            window.draw(Exit);
            SetMute();
            window.draw(Mute);
            DrawCursor();
            Sleep(0.1);
            window.display();
        }
        while (ShowPass());
        std::remove("SaveFile.txt");
        return true;
    }
    return false;
}

bool ShowPass()
{
    window.clear();
    DrawBoard();
    window.draw(Pass);
    Exit.setPosition(Offset.Xcord + 425, 700);
    if (Mouse::getPosition(window).x >= Offset.Xcord + 425 && Mouse::getPosition(window).x <= Offset.Xcord + 575 && Mouse::getPosition(window).y >= 700 && Mouse::getPosition(window).y <= 850)
    {
        Exit.setScale(0.33, 0.33);
        if (Mouse::isButtonPressed(Mouse::Left) && IsMouseDown == false)
        {
            IsMouseDown = true;
            if (Enable_Audio) PlayButtonPressSound();
            Exit.setPosition(20, 20);
            GameState = 0;
            return false;
        }
    }
    else Exit.setScale(0.3, 0.3);
    window.draw(Exit);
    SetMute();
    window.draw(Mute);
    DrawCursor();
    window.display();
    while (window.pollEvent(e))
    {
        if (e.type == Event::Closed)window.close();
        if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
    }
    return true;
}

void ShowTime()
{
    if (ShouldUpdateTimer) TimeElapsed = MyClock.getElapsedTime();
    int time = TimePerMove - TimeElapsed.asSeconds(); //time remaining

    if (time == 0) IsTimeOut = true;

    if (time < 10)
    {
        Nums.setTextureRect(IntRect(time * 50, 0, 50, 75));
        Nums.setPosition(197.5, 450);
        window.draw(Nums);
        if (Enable_Audio && Moves != 0 && Score < Target) PlayTimeOutSound(time);
    }
    else if (time < 100)
    {
        Nums.setTextureRect(IntRect((time / 10) * 50, 0, 50, 75));
        Nums.setPosition(172.5, 450);
        window.draw(Nums);
        Nums.setTextureRect(IntRect((time % 10) * 50, 0, 50, 75));
        Nums.setPosition(222.5, 450);
        window.draw(Nums);
    }
}

void SaveToFile()
{
    std::ofstream fout("SaveFile.txt");
    fout << Tiles << "\n" << Moves << "\n" << Score << "\n" << Target << "\n" << Enable_Diagonals << "\n" << Enable_Elbows << "\n" << Hints_Used << "\n" << ShouldGiveHints << "\n";
    for (int i = 0; i < Tiles; i++) for (int j = 0; j < Tiles; j++) fout << board[i][j] << " ";
    fout.close();
}

void LoadFile()
{
    std::ifstream fin("SaveFile.txt");
    if (fin.fail())
    {
        if (Enable_Audio) PlaySwapSound(false);
        return;
    }
    if (Enable_Audio) PlayLevelStart();
    fin >> Tiles >> Moves >> Score >> Target;
    Tilesize = 1000.0 / Tiles;
    fin >> Enable_Diagonals >> Enable_Elbows >> Hints_Used >> ShouldGiveHints;
    for (int i = 0; i < Tiles; i++) for (int j = 0; j < Tiles; j++) fin >> board[i][j];
    GameState = 1;
    if (Tiles == 5) Background.setTextureRect(IntRect(0, 1080, 1920, 1080));
    else if (Tiles == 6) Background.setTextureRect(IntRect(1920, 0, 1920, 1080));
    else if (Tiles == 7)  Background.setTextureRect(IntRect(3840, 1080, 1920, 1080));
    else if (Tiles == 8)  Background.setTextureRect(IntRect(1920, 1080, 1920, 1080));
    else if (Tiles == 9)  Background.setTextureRect(IntRect(3840, 0, 1920, 1080));
    else  Background.setTextureRect(IntRect(0, 0, 1920, 1080));
    fin.close();
}

void BackgroundMusic()
{
    MyMusic.openFromFile("Resources/Song.ogg");
    MyMusic.setVolume(10);
    MyMusic.setLoop(true);
    if (Enable_Audio)MyMusic.play();
}

void PlayRandomSound()
{
    int temp = rand() % 6;
    SoundEffect.resetBuffer();
    if (temp == 0) SoundEffect.setBuffer(Delicious);
    else if (temp == 1) SoundEffect.setBuffer(Divine);
    else if (temp == 2) SoundEffect.setBuffer(Frogtastic);
    else if (temp == 3) SoundEffect.setBuffer(SugarCrush);
    else if (temp == 4) SoundEffect.setBuffer(Sweet);
    else SoundEffect.setBuffer(Tasty);
    SoundEffect.play();
}

void PlayLevelPass()
{
    SoundEffect.resetBuffer();
    SoundEffect.setBuffer(Level_Completed);
    SoundEffect.play();
}

void PlayLevelFail()
{
    SoundEffect.resetBuffer();
    SoundEffect.setBuffer(Level_Failed);
    SoundEffect.play();
}

void PlayLevelStart()
{
    SoundEffect2.resetBuffer();
    SoundEffect2.setBuffer(Level_Started);
    SoundEffect2.play();
}

void DrawProgressBar()
{
    ProgressBar.setTextureRect(IntRect(0, 0, (Score * 324.0 / Target) + 38, 800));
    window.draw(ProgressBar);
}

void DrawHintNum()
{
    Nums.setTextureRect(IntRect((HintsLimit - Hints_Used) * 50, 0, 50, 75));
    Nums.setPosition(1725, 20);
    window.draw(Nums);
}

void PlayTimeOutSound(int time)
{
    if (time <= 5 && SoundEffect3.getStatus() == 0 && SoundEffect3.getStatus() != 2 && SoundEffect3.getStatus() != 1 && Enable_Audio) SoundEffect3.play();
}

bool PopSpecialCandies()
{
    double speed = 5;
    int currentI, currentJ;
    if (board[Current_Tile_I][Current_Tile_J] == 6 || board[Current_Tile_I][Current_Tile_J] == 7)
    {
        currentI = Current_Tile_I;
        currentJ = Current_Tile_J;
    }
    else
    {
        currentI = Last_Tile_I;
        currentJ = Last_Tile_J;
    }
    if (board[currentI][currentJ] >= 6) //Special Candy Pop
    {
        SoundEffect5.resetBuffer();
        SoundEffect5.setBuffer(Special);
        if (Enable_Audio) SoundEffect5.play();
        if (board[currentI][currentJ] == 6 && IsUserSwap) //Disc
        {
            int left = 1, right = 1, drop = 3;
            if (currentI == 0) left = 0;
            else if (currentI == Tiles - 1) right = 0;
            if (currentJ == 0 || currentJ == Tiles - 1) drop--;
            for (double k = 0; k <= Tilesize * drop; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(i >= currentI - left && i <= currentI + right && j >= currentJ - 1 && j <= currentJ + 1))
                        {
                            if (i >= currentI - left && i <= currentI + right && j < currentJ - 1) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;

            }
            for (int i = currentI - left; i <= currentI + right; i++) if (currentJ > 0) for (int j = currentJ - 2; j >= 0; j--) board[i][j + drop] = board[i][j];
            for (int i = currentI - left; i <= currentI + right; i++) for (int j = 0; j <= drop - 1; j++) board[i][j] = rand() % 6;
            speed = 20;
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();
                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (i >= currentI - left && i <= currentI + right && j <= drop - 1) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
            Score += drop;
            if (left) Score += drop;
            if (right)Score += drop;
        }
        else if (board[currentI][currentJ] == 7 && IsUserSwap)//Bomb
        {
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();

                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        if (!(i == currentI || j == currentJ))
                        {
                            if (i != currentI && j < currentJ) Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord + k);
                            else Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                            Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                            window.draw(Candy);
                        }
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;

            }
            for (int i = 0; i < Tiles; i++) if (i != currentI) for (int j = currentJ - 1; j >= 0; j--) board[i][j + 1] = board[i][j];
            for (int i = 0; i < Tiles; i++) board[i][0] = rand() % 6;
            for (int j = 0; j < Tiles; j++) board[currentI][j] = rand() % 6;
            speed = 20;
            for (double k = 0; k <= Tilesize; k += Tilesize / speed)
            {
                window.clear();
                DrawComponents();
                //Draw Candies
                for (int i = 0; i < Tiles; i++)
                {
                    for (int j = 0; j < Tiles; j++)
                    {
                        Candy.setPosition((i * Tilesize) + Offset.Xcord, (j * Tilesize) + Offset.Ycord);
                        if (i == currentI || j == 0) Candy.setColor(Color(255, 255, 255, std::min(static_cast<int>((k * speed / Tilesize + 1) * 255 / speed), 255)));
                        else Candy.setColor(Color(255, 255, 255, 255));
                        Candy.setTextureRect(IntRect(board[i][j] * 760, 0, 760, 760));
                        window.draw(Candy);
                    }
                }
                //Draw Cursor
                DrawCursor();
                window.display();
                Sleep(2);
                SetMute();
                if (ExitStage()) return false;
                while (window.pollEvent(e)) if (e.type == Event::MouseButtonReleased) IsMouseDown = false;
            }
            Score += 2 * Tiles - 1;
        }
        return true;
    }
    return false;
}