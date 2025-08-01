#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <cstdlib>
#include<fstream>
#include<sstream>

using namespace sf;
using namespace std;

//AYESHA NOOR
//23I-0736
//CS B

int GRID_CELL_WIDTH = 48;
int GRID_CELL_HEIGHT = 48;

//menu class to display the main menu
class menu
{
public:
    void display(RenderWindow& window, Texture& main)
    {
        Sprite s_map;
        s_map.setTexture(main);
        s_map.setPosition(0, 0);
        window.draw(s_map);
    }

    void displayPlayButton(RenderWindow& window, Texture& play)
    {
        Sprite s_map;
        s_map.setTexture(play);
        s_map.setPosition(100, 700);
        window.draw(s_map);
    }

    void displaySaveButton(RenderWindow& window, Texture& saveTexture)
    {
        Sprite s_map;
        s_map.setTexture(saveTexture);
        s_map.setPosition(460, 700);
        window.draw(s_map);
    }

    void displayLoadButton(RenderWindow& window, Texture& loadTexture) 
    {
        Sprite s_map;
        s_map.setTexture(loadTexture);
        s_map.setPosition(760, 700);
        window.draw(s_map);
    }

    void displayExitButton(RenderWindow& window, Texture& exitTexture)
    {
        Sprite s_map;
        s_map.setTexture(exitTexture);
        s_map.setPosition(1060, 700);
        window.draw(s_map);
    }

    void displayWinScreen(RenderWindow& window, Texture& winTexture)
    {
        Sprite s_map;
        s_map.setTexture(winTexture);
        s_map.setPosition(0, 0);
        window.draw(s_map);
    }

    void displayLoseScreen(RenderWindow& window, Texture& loseTexture)
    {
        Sprite s_map;
        s_map.setTexture(loseTexture);
        s_map.setPosition(0, 0);
        window.draw(s_map);
    }
};

//every entity has coordinates, thus a separate class for reuseablity
struct coordinates 
{
    int x;
    int y;
    coordinates() : x(0), y(0) {}
    coordinates(int setX, int setY) : x(setX), y(setY) {}
};

//composition w coordinates
class Entity 
{
protected:
    coordinates coord;
    bool isAlive;
public:
    Entity(bool b = true) : isAlive(b)
    {
    }
    virtual ~Entity()                   //  polymorphic behaviour for each child class
    {
    }

    //getters setters
    bool get_health() const
    {
        return isAlive;
    }

    void set_health(bool new_health) 
    {
        isAlive = new_health;
    }

    void setpos(int x, int y)
    {
        this->coord.x = x;
        this->coord.y = y;
    }

    int getx() const
    {
        return this->coord.x;
    }

    int gety() const 
    {
        return this->coord.y;
    }
};

//composed of coordinates cuz position
//timer indicating after how many seconds it should explode
class Bomb
{
private:
    coordinates pos;
    int timer;
    Clock clock;
    bool exploded;
    Time explosionTime; // Time when the bomb exploded

public:
    Bomb(int x = 0, int y = 0, int t = 1) : pos(x, y), timer(t), exploded(false)
    {
        clock.restart();
    }

    void setpos(int x, int y)
    {
        this->pos.x = x;
        this->pos.y = y;
        clock.restart();
        exploded = false;
    }

    int getx() const
    {
        return this->pos.x;
    }

    int gety() const
    {
        return this->pos.y;
    }

    sf::Time explosionDuration = sf::seconds(1);
    sf::Time elapsed = clock.getElapsedTime();

    bool isExploded()
    {
        if (!exploded && clock.getElapsedTime().asSeconds() >= timer)
        {
            exploded = true;
            explosionTime = clock.getElapsedTime(); // Capture explosion time
        }
        return exploded && clock.getElapsedTime() <= (explosionTime + explosionDuration); // Keep explosion visible for 1 second
    }
};

//child class of entity
class player : public Entity 
{
public:
    Bomb bombs[10];                   //an array but they are restored when exploded, so unlimited
    int bombCount;

    player() : Entity(), bombCount(0)
    {    }

    void dropBomb(char playergrid[17][29])
    {
        if (bombCount < 10) {
            int bx = getx();
            int by = gety();
            bombs[bombCount].setpos(bx, by);
            bombCount++;
            playergrid[bx][by] = 'b';  // Update grid with bomb position
        }
    }
    bool ishurdle = 1;                     //this is for the reveal of the hidden exit door
    
    bool gethurdle()
    {
        return ishurdle;
    }

    void updateBombs(char playergrid[17][29], Entity* enemies, int numEnemies)
    {
        for (int i = 0; i < bombCount; ++i)
        {
            if (bombs[i].isExploded()) {
                int bx = bombs[i].getx();
                int by = bombs[i].gety();

                // update the grid for explosion 
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        int new_x = bx + dx;
                        int new_y = by + dy;

                        // indexing within bounds
                        if (new_x >= 0 && new_x < 17 && new_y >= 0 && new_y < 29)
                        {
                            if (playergrid[new_x][new_y] == 'h')
                            {
                                playergrid[new_x][new_y] = ' '; // destroy if hurdle

                                // check if the destroyed hurdle had hidden exit behind it
                                if (new_x == 10 && new_y == 12)
                                {
                                    ishurdle = 0;
                                    playergrid[10][12] = 'F'; // revealing the exit
                                }
                            }

                            else if (playergrid[new_x][new_y] == 'E')
                            {
                                playergrid[new_x][new_y] = ' '; // destroy the enemy at that position

                                for (int i = 0; i < 7; i++)
                                {
                                    if (enemies[i].getx() == new_x && enemies[i].gety() == new_y)
                                    {
                                        enemies[i].set_health(0);
                                    }
                                }
                            }

                            else if (playergrid[new_x][new_y] == 'P')          // player dies if doesnt move after placing a bomb
                            {
                                this->set_health(0);
                            }
                        }
                    }
                }

                // free grid from bomb after explosion
                playergrid[bx][by] = ' ';
                bombs[i] = bombs[--bombCount];
                --i; // re fixing index
            }
        }
    }
};

//child class 2 from entity 
class enemy : public Entity 
{
public:
    int direction;
    float speed;

    enemy() : Entity(), direction(0), speed(0.9f)
    {
        direction = rand() % 4;      // 0 = left, 1 = right, 2 = up, 3 = down
    }

    //random movement of enemies if they collide w something
    void moveEnemy(int rows, int cols, char playergrid[17][29]) 
    {
        if (get_health())
        {
            int dx = 0, dy = 0;
            switch (direction) {
            case 0: dy = -1; break; // left
            case 1: dy = 1; break;  // right
            case 2: dx = -1; break; // up
            case 3: dx = 1; break;  // down
            }
            int new_x = getx() + dx;
            int new_y = gety() + dy;

            // checks enemy doesn't go out of indexs or collide with walls or hurdles or the exit door
            if (new_x > 0 && new_x < rows - 1 && new_y > 0 && new_y < cols - 1 &&
                playergrid[new_x][new_y] != 'w' && playergrid[new_x][new_y] != 'h' && playergrid[new_x][new_y] != 'b' && playergrid[new_x][new_y] != 'E' && playergrid[new_x][new_y] != 'F') 
            {
                setpos(new_x, new_y);
            }
            else   //if it is, change direction
            {
                direction = rand() % 4; 
            }
        }
    }
};

//the main class of game screen
//has 2d array of boxes, 7 enemies for level 1 & has a player
class PlayerGrid
{
private:
    const int rows = 17;
    const int cols = 29;
    Entity*** grid;


public:
    player p;
    char playergrid[17][29];
   // int spritesarray[20][20];
    bool drawn = true;
    enemy enemies[7]; // to store multiple enemies


    PlayerGrid() 
    {
        // player position to top left 
        p.setpos(1, 1);
        p.set_health(1);
         
        //setting player on grid
        for (int i = 0; i < rows; i++) 
        {
            for (int j = 0; j < cols; j++) 
            {
                if (i == p.getx() && j == p.gety()) 
                {
                    playergrid[i][j] = 'P';
                }
                else 
                {
                    playergrid[i][j] = ' ';
                }
            }
        }

        //constructing entity grid
        grid = new Entity * *[rows];
        for (int i = 0; i < rows; ++i) 
        {
            grid[i] = new Entity * [cols];
            for (int j = 0; j < cols; ++j)
            {
                grid[i][j] = nullptr;
            }
        }
    }

    ~PlayerGrid()               //de allocating memory
    {
        for (int i = 0; i < rows; ++i) 
        {
            delete[] grid[i];
        }
        delete[] grid;
    }

    //the permanent ones    represented by w (for walls)
    void createWalls()
    {
        // Initialize the borders
        for (int i = 0; i < cols; i++) 
        {
            playergrid[0][i] = 'w';
            playergrid[16][i] = 'w';
        }

        for (int i = 0; i < rows; i++)
        {
            playergrid[i][0] = 'w';
            playergrid[i][28] = 'w';
        }

        // Fixed walls
        for (int i = 5; i < 8; i++) 
        {
            playergrid[3][i] = 'w';
            playergrid[4][i] = 'w';
            playergrid[5][i] = 'w';
            playergrid[6][i] = 'w';
            playergrid[7][i] = 'w';
        }

        for (int i = 16; i < 23; i++)
        {
            playergrid[8][i] = 'w';
        }

        playergrid[7][6] = 'w';
        playergrid[15][19] = 'w';
        playergrid[11][22] = 'w';
        playergrid[9][3] = 'w';
        playergrid[10][12] = 'w';
        playergrid[16][6] = 'w';
        playergrid[5][19] = 'w';
        playergrid[1][22] = 'w';
        playergrid[13][3] = 'w';
        playergrid[4][12] = 'w';

        playergrid[11][12] = 'w';
        playergrid[12][12] = 'w';
        playergrid[13][12] = 'w';
        playergrid[14][12] = 'w';
        playergrid[11][13] = 'w';
        playergrid[12][13] = 'w';
        playergrid[13][13] = 'w';
        playergrid[14][13] = 'w';

        playergrid[7][24] = 'w';
        playergrid[8][23] = 'w';
        playergrid[7][24] = 'w';
        playergrid[8][23] = 'w';
    }

    //creating walls that can be destroyed          hurdles (represented by h)
    void createhurdles() 
    {
        for (int i = 9; i < 15; i++) 
        {
            playergrid[3][i + 1] = 'h';
            playergrid[10][i + 2] = 'h';
            playergrid[12][i] = 'h';
            playergrid[5][i] = 'h';
            playergrid[9][i] = 'h';
        }

        for (int i = 12; i < 18; i++)
        {
            playergrid[4][i] = 'h';
        }


        playergrid[1][7] = 'h';
        playergrid[3][1] = 'h';
        playergrid[4][3] = 'h';
        playergrid[7][7] = 'h';
        playergrid[15][20] = 'h';
        playergrid[10][22] = 'h';
        playergrid[10][3] = 'h';
        playergrid[10][11] = 'h';
        playergrid[10][12] = 'h';
        playergrid[10][13] = 'h';
        playergrid[2][25] = 'h';
        playergrid[10][3] = 'h';
        playergrid[6][10] = 'h';

        playergrid[10][19] = 'h';
        playergrid[13][12] = 'h';
        playergrid[15][10] = 'h';
        playergrid[15][12] = 'h';
        playergrid[15][13] = 'h';
        playergrid[13][13] = 'h';
        playergrid[11][13] = 'h';
        playergrid[10][13] = 'h';

        playergrid[7][12] = 'h';
        playergrid[8][15] = 'h';
        playergrid[7][19] = 'h';
        playergrid[8][19] = 'h';

        playergrid[3][19] = 'h';
        playergrid[5][12] = 'h';
        playergrid[9][10] = 'h';
        playergrid[12][12] = 'h';
        playergrid[15][1] = 'h';
        playergrid[14][3] = 'h';
        playergrid[12][23] = 'h';
        playergrid[1][13] = 'h';
    }

    //create enemies on random position. 
    void createenemies()
    {
        int nx, ny;

        for (int i = 0; i < 7; i++) 
        {
            do 
            {
                nx = 1 + rand() % (rows - 2);  // cnsure enemies are placed within grid bounds
                ny = 1 + rand() % (cols - 2);
            } while (playergrid[nx][ny] != ' ');  // check that the position is empty

            enemies[i].setpos(nx, ny);
            playergrid[nx][ny] = 'E';
        }
    }


    //de bugging / terminal game
    void display() 
    {
        system("cls");   //clearing console

        for (int i = 0; i < rows; i++) 
        {
            for (int j = 0; j < cols; j++) 
            {
                cout << playergrid[i][j] << " ";
            }
            cout << endl;
        }
    }

    void updatePlayerPosition(int dx, int dy) 
    {
        if (!p.get_health()) 
            return; // do nothing if the player is dead

        int new_x = p.getx() + dx;
        int new_y = p.gety() + dy;

        // check boundaries and collisions
        //player cant walk over walls, hurdles, or bombs
        if (new_x > 0 && new_x < rows - 1 && new_y > 0 && new_y < cols - 1 && playergrid[new_x][new_y] != 'w' && playergrid[new_x][new_y] != 'h' && playergrid[new_x][new_y] != 'b') 
        {
            if (playergrid[p.getx()][p.gety()] != 'b')
            { 
                // Clear old position if it's not a bomb
                playergrid[p.getx()][p.gety()] = ' ';
            }

            p.setpos(new_x, new_y);

            //dying condition
            if (playergrid[new_x][new_y] == 'E') 
            {
                p.set_health(0); //  dies if touching an enemy
                playergrid[new_x][new_y] = ' ';
              //  cout << "DEAD." << endl;
            }
            else
            {
                playergrid[new_x][new_y] = 'P'; // set new position
            }
        }
    }

    //enemy positions (always changing)
    void updateEnemyPositions()
    {
        if (!p.get_health()) 
            return; // Do nothing if the player is dead

        for (int i = 0; i < 7; ++i)
        {
            if (enemies[i].get_health())
            {
                playergrid[enemies[i].getx()][enemies[i].gety()] = ' '; // clearing old position
                enemies[i].moveEnemy(rows, cols, playergrid);
                if (enemies[i].getx() == p.getx() && enemies[i].gety() == p.gety())
                {
                    p.set_health(0); // player dies if collides w an enemy
                   // cout << "DEAD." << endl;
                    playergrid[enemies[i].getx()][enemies[i].gety()] = ' ';
                    // break; // No need to check further if the player is dead
                }
                playergrid[enemies[i].getx()][enemies[i].gety()] = 'E'; // new position updating grid
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------------------
    //BELOW ARE ONLY THE DISPLAY FUNCTIONS FOR THE GRID
    void displayWalls(RenderWindow& window, Texture& wallTexture) 
    {
        Sprite s_map;
        s_map.setTexture(wallTexture);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (playergrid[i][j] == 'w') {
                    s_map.setPosition(j * GRID_CELL_WIDTH, i * GRID_CELL_HEIGHT);
                    window.draw(s_map);
                }
            }
        }
    }

    void displayExit(RenderWindow& window, Texture& exitTexture) {
        Sprite s_map;
        s_map.setTexture(exitTexture);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (playergrid[i][j] == 'F') {
                    s_map.setPosition(j * GRID_CELL_WIDTH, i * GRID_CELL_HEIGHT);
                    window.draw(s_map);
                }
            }
        }
    }

    void displayhurdles(RenderWindow& window, Texture& hurdlesTexture) {
        Sprite s_map;
        s_map.setTexture(hurdlesTexture);

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (playergrid[i][j] == 'h') {
                    s_map.setPosition(j * GRID_CELL_WIDTH, i * GRID_CELL_HEIGHT);
                    window.draw(s_map);
                }
            }
        }
    }

    void displayPlayer(RenderWindow& window, Sprite& playerSprite)
    {
        if (p.get_health())
        {
            playerSprite.setPosition(p.gety() * GRID_CELL_WIDTH, p.getx() * GRID_CELL_HEIGHT);
            window.draw(playerSprite);
        }
    }

    void displayEnemies(RenderWindow& window, Sprite& enemySprite)
    {
        for (int i = 0; i < 7; ++i)
        {
            if (enemies[i].get_health())
            {
                enemySprite.setPosition(enemies[i].gety() * GRID_CELL_WIDTH, enemies[i].getx() * GRID_CELL_HEIGHT);
                window.draw(enemySprite);
            }
        }
    }

    void displaybombs(RenderWindow& window, Texture& bombTexture)
    {
        Sprite s_map;
        s_map.setTexture(bombTexture);

        for (int i = 0; i < 17; i++) {
            for (int j = 0; j < 29; j++) {
                if (playergrid[i][j] == 'b') {
                    s_map.setPosition(j * GRID_CELL_WIDTH, i * GRID_CELL_HEIGHT);
                    window.draw(s_map);
                }
            }
        }
    }

    void displayexplosion(RenderWindow& window, Texture& eTexture)
    {
        Sprite s_map;
        s_map.setTexture(eTexture);

        for (int i = 0; i < 17; i++) {
            for (int j = 0; j < 29; j++) {
                if (playergrid[i][j] == 'b') {
                    s_map.setPosition(j * GRID_CELL_WIDTH, i * GRID_CELL_HEIGHT);
                    window.draw(s_map);
                }
            }
        }
    }

    //-----------------------------------------------------------------------------------------------------------------------------------

    //winning if reaching hidden exit
    bool checkWinCondition() 
    {
        if (!p.gethurdle() && p.getx() == 10 && p.gety() == 12)
            return 1;
        else
            return 0; 
    }

    //if play button is clicked
    void reset()
    {
        createenemies();
        createWalls();
        createhurdles();
    }

    //file hand;ing part
    void saveGameState(const std::string& filename) 
    {
        std::ofstream file(filename);

        if (file.is_open()) 
        {
            // Save player position and health
            file << "player's position" << endl;
            file << p.getx() << " " << p.gety() << endl;
            file << "player's health" << endl;
            file<< p.get_health() << std::endl;


            file << "enemies position and health : " << endl;
            // Save enemies' positions and health
            for (int i = 0; i < 7; ++i) 
            {
                file << "enemy : " << i + 1 << " : ";
                file << enemies[i].getx() << " " << enemies[i].gety() << " " << enemies[i].get_health() << std::endl;
            }

            file << "bombs position and state : " << endl;

            // Save bombs' positions and states
            for (int i = 0; i < p.bombCount; ++i) 
            {
                file << p.bombs[i].getx() << " " << p.bombs[i].gety() << " " <<p.bombs[i].isExploded() << std::endl;
            }

            file.close();
        }
    }

    // Function to load the game state
    void loadGameState(const std::string& filename) 
    {
        std::ifstream file(filename);

        if (file.is_open())
        {
            int px, py, pHealth;
            file >> px >> py >> pHealth;
            p.setpos(px, py);
            p.set_health(pHealth);

            for (int i = 0; i < 7; ++i)
            {
                int ex, ey, eHealth;
                file >> ex >> ey >> eHealth;
                enemies[i].setpos(ex, ey);
                enemies[i].set_health(eHealth);
            }

            // Clear previous bombs and reinitialize
            p.bombCount = 0;
            char grid[17][29];
            for (int i = 0; i < 17; ++i)
            {
                for (int j = 0; j < 29; ++j) 
                {
                    grid[i][j] = ' ';
                }
            }

            int bx, by;
            bool exploded;
            while (file >> bx >> by >> exploded) {
                p.bombs[p.bombCount] = Bomb(bx, by, 1);
                p.bombs[p.bombCount].setpos(bx, by);
                p.bombCount++;
            }

            cout << p.getx() << "," << p.gety();

          

            file.close();
        }
    }


};

//background of the game
void createBack(RenderWindow& window, Texture& backgroundTexture) 
{
    Sprite s_map;
    s_map.setTexture(backgroundTexture);
    s_map.setPosition(0, 0);
    window.draw(s_map);
}

//----------------------------- M A I N    F U N C T I O N --------------------------------------------------------
int main()
{
    //font for timer display
    Font font;
    if (!font.loadFromFile("C:\\Users\\Administrator\\Downloads\\font.ttf")) {
        return -1; // Handle font loading failure
    }
    Text timeText;
    timeText.setFont(font);
    timeText.setCharacterSize(20); // Set font size
    timeText.setFillColor(Color::Yellow); // Set text color
    timeText.setPosition(10, 10); // Set position on the screen
    //time
    srand(static_cast<unsigned>(time(0)));
    //creating window
    RenderWindow window(VideoMode(1390, 820), "BOMBERMAN");
    //selecting icon
    Image icon;
    icon.loadFromFile("C:\\Users\\Administrator\\Downloads\\Bomberman-32x32.png");
    window.setIcon(32, 32, icon.getPixelsPtr());

    //textures for menu
    Texture menuTexture, playTexture, winTexture, loseTexture, saveTexture, loadTexture, exitTexture;
    if (!menuTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\menu (1).jpg") ||
        !playTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\play.jpg") ||
        !winTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\you_win.png") ||
        !loseTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\you_lose.png") ||
        !saveTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\save.png") ||
        !loadTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\load.png") ||
        !exitTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\exitmenu.png")) {
        return -1;
    }

    //textures for when game is running
    Texture wallTexture, backgroundTexture, playerTexture, hurdlesTexture, enemyTexture, bombTexture, doorTexture, explosionTexture;
    if (!wallTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\WhatsApp Image 2024-07-27 at 10.24.10 PM.jpg") ||
        !backgroundTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\bg11.png") ||
        !playerTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\player1-removebg-preview.png") ||
        !hurdlesTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\hurdle4.png") ||
        !enemyTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\enemy-removebg-preview.png") ||
        !bombTexture.loadFromFile("C:\\Users\\Administrator\\Downloads\\bomb-removebg-preview.png") ||
        !doorTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\exit.png") ||
        !explosionTexture.loadFromFile("C:\\Users\\Administrator\\Pictures\\exit.png")) {
        return -1;
    }

    //bg music in hope of a bonus :)
    sf::Music backgroundMusic;
    if (!backgroundMusic.openFromFile("C:\\Users\\Administrator\\Downloads\\Centipede_Skeleton\\Music\\field_of_hopes.ogg"))
    {
        return -1;
    }

    //body for player and enemy
    Sprite player(playerTexture);
    Sprite enemy(enemyTexture);

    menu m;
    bool gameRunning = false;
    bool gameWon = false;
    bool gameLost = false;

    PlayerGrid playergridobj;

    Clock gameClock; // Clock for game logic
    Clock timerClock; // Separate clock for displaying time
    Time enemyMoveInterval = seconds(0.5f);

    backgroundMusic.setLoop(true); // Loop the music
    backgroundMusic.play(); // Start playing the music

    int x = 0;          //this var tracks the death of enemies, if its 7, meaning all enemies died, the player wins

    while (window.isOpen())
    {

        playergridobj.display();       //terminal game

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            //menu part
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                Vector2i mousePos = Mouse::getPosition(window);
                if (!gameRunning)
                {
                    // Check for menu button clicks
                    if (mousePos.x >= 100 && mousePos.x <= 100 + playTexture.getSize().x &&
                        mousePos.y >= 700 && mousePos.y <= 700 + playTexture.getSize().y) 
                    {
                        gameRunning = true;
                        gameWon = false;
                        gameLost = false;
                        playergridobj.reset();
                        timerClock.restart(); // Restart the timer clock
                    }
                    else if (mousePos.x >= 460 && mousePos.x <= 460 + saveTexture.getSize().x &&
                        mousePos.y >= 700 && mousePos.y <= 700 + saveTexture.getSize().y)
                    {
                        playergridobj.saveGameState("C:\\Users\\Administrator\\Desktop\\gamestate.txt");
                    }
                    else if (mousePos.x >= 760 && mousePos.x <= 760 + loadTexture.getSize().x &&
                        mousePos.y >= 700 && mousePos.y <= 700 + loadTexture.getSize().y) 
                    {
                        playergridobj.loadGameState("C:\\Users\\Administrator\\Desktop\\gamestate.txt");
                        gameRunning = true;
                        timerClock.restart(); // Restart the timer clock
                    }
                    else if (mousePos.x >= 1060 && mousePos.x <= 1060 + exitTexture.getSize().x &&
                        mousePos.y >= 700 && mousePos.y <= 700 + exitTexture.getSize().y) 
                    {
                        window.close();
                    }
                }
                else
                {
                    // Check for play button clicks on win/lose screen
                    if ((gameWon || gameLost) &&
                        mousePos.x >= 200 && mousePos.x <= 200 + playTexture.getSize().x &&
                        mousePos.y >= 700 && mousePos.y <= 700 + playTexture.getSize().y) 
                    {
                        gameRunning = true;
                        gameWon = false;
                        gameLost = false;
                        playergridobj.reset();
                        timerClock.restart(); // Restart the timer clock
                    }
                }
            }

            //handling movement of the player
            if (event.type == Event::KeyPressed) 
            {
                if (event.key.code == Keyboard::Left) 
                {
                    playergridobj.updatePlayerPosition(0, -1);
                }
                else if (event.key.code == Keyboard::Right)
                {
                    playergridobj.updatePlayerPosition(0, 1);
                }
                else if (event.key.code == Keyboard::Up)
                {
                    playergridobj.updatePlayerPosition(-1, 0);
                }
                else if (event.key.code == Keyboard::Down)
                {
                    playergridobj.updatePlayerPosition(1, 0);
                }
                else if (event.key.code == Keyboard::B)          //placing bomb
                {
                    playergridobj.p.dropBomb(playergridobj.playergrid);
                }
            }
        }

        //while game runs
        if (gameRunning)
        {
            if (gameClock.getElapsedTime() >= enemyMoveInterval)
            {
                playergridobj.updateEnemyPositions();
                gameClock.restart(); // Restart the game clock
            }
            //displaying the grid
            window.clear();
            createBack(window, backgroundTexture);
            playergridobj.displayWalls(window, wallTexture);
            playergridobj.displayhurdles(window, hurdlesTexture);
            playergridobj.displaybombs(window, bombTexture);
            playergridobj.displayExit(window, doorTexture);
            playergridobj.displayPlayer(window, player);
            playergridobj.displayEnemies(window, enemy);
            playergridobj.p.updateBombs(playergridobj.playergrid, playergridobj.enemies, 7);


            //winning and losing conditions
            if (playergridobj.checkWinCondition()) 
            {
                gameWon = true;
                gameRunning = false;
            }
            if (!playergridobj.p.get_health()) 
            {
                gameLost = true;
                gameRunning = false;
            }
        }

        //if game aint running rn
        else
        {
            window.clear();
            m.display(window, menuTexture);
            if (gameWon)
                m.displayWinScreen(window, winTexture);
            if (gameLost)
                m.displayLoseScreen(window, loseTexture);
            m.displayPlayButton(window, playTexture);
            m.displaySaveButton(window, saveTexture);
            m.displayLoadButton(window, loadTexture);
            m.displayExitButton(window, exitTexture);
        }

        //if all enemies died
        if (x == 7)
        {
            gameWon = 1;
        }
        //checing upper condition everytime the loop renews
        for (int i = 0; i < 7; i++)
        {
            if (playergridobj.enemies[i].get_health() == 0)
                x++;
        }
        //displaying time at top left & update the text to display the elapsed time from timerClock
        Time elapsedTime = timerClock.getElapsedTime();
        float seconds = elapsedTime.asSeconds();
        std::ostringstream timeStream;
        timeStream << "Time: " << seconds<<" s";
        timeText.setString(timeStream.str());
        window.draw(timeText);
        window.display();
    }
    return 0;
}