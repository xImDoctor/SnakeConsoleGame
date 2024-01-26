#include <iostream>

#include <Windows.h>
#include <MMSystem.h> //play music

#include <deque>
#include <ctime>
#include <cstdlib>
#include <conio.h>


#define SET_DEFAULT_COLOR SetTextColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define YELLOW_COLOR FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define MAGENTA FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY

using namespace std;

//void but remade for work with knock-out check
void MoveToXY(int x, int y) {

	COORD pos = { x, y };
	HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(output, pos);
}

void SetTextColor(int color) {

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
}

//window parameters
//board Y parameters
const unsigned short stringsABOVE = 2; //score string = 1; + max score string = 1; previous just 1
const unsigned short stringsUNDER = 1; //string under game field
const unsigned int stringsAtALL = stringsABOVE + stringsUNDER;

const unsigned short inFieldHeight = 7;  //game field height

//board X parameters
const unsigned short inFieldWidth = 16;
const unsigned short LEFTbound = 1;
const unsigned short RIGHTbound = 2;
const unsigned short Xbound = LEFTbound + RIGHTbound;


//full display
//const unsigned short HEIGHT = 10; //fieldHeight + stringsABOVE + fieldHeight; // height = field + ABOVE + UNDER
const unsigned short HEIGHT = inFieldHeight + stringsABOVE + stringsUNDER;
const unsigned short WIDTH = inFieldWidth + Xbound; // from # to # + \n
const unsigned short windowUpdateTime = 200; //ms


const char map[] = { 
	"##################\n" //19
	"#................#\n"
	"#................#\n"
	"#................#\n"
	"#................#\n"
	"#................#\n"
	"#................#\n"
	"##################\n"
	//8
};

COORD appleCoords;
const char apple = 'a';

const char snakeSymb = '0';
const unsigned short snakeStartLen = 2;
unsigned short snakeLen, score, maxScore = 0;

enum DIRECTION {NONE, LEFT, RIGHT, UP, DOWN};
char curDirection, newDirection;

//function templates
char checkDirection();
char changeDirection(char&, char);
void snakeMove(char, deque<COORD>&);

bool isKnocked(const COORD&);
bool isBitenItself( deque<COORD>&);
bool isEaten(COORD&, const COORD&);
void spawnApple(COORD&, const deque<COORD>&);

void GameOver(bool&);
void GameLoop(bool& _gameOver) {
	snakeLen = snakeStartLen;
	score = 0;

	curDirection = RIGHT, newDirection = NONE;
	srand(static_cast<unsigned int>(time(0)));

	deque<COORD> snake; //deque to store coord of every snake point
	/*
		snakeCoords.X = WIDTH / 2;
		snakeCoords.Y = HEIGHT / 2;
		//first point coords is a middle of the window
		//coord from Windows.h
	*/
	//first snake's element to the middle of game field
	snake.push_back({ WIDTH / 2, HEIGHT / 2 });
	spawnApple(appleCoords, snake);

	// start apple pos
	// can be in any place of game field, but not under the snake (not in the center)
	/*
	do 
	{
		appleCoords.X = rand() % (WIDTH - Xbound) + 1;
		appleCoords.Y = rand() % (HEIGHT - 1 - stringsAtALL) + stringsAtALL;

	} while (appleCoords.X == (WIDTH / 2) && appleCoords.Y == (HEIGHT / 2));
	*/

	PlaySound(TEXT("sound/toreador.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	while (!_gameOver) {
		Sleep(windowUpdateTime);

		
		MoveToXY(0, 0); SET_DEFAULT_COLOR;
		cout << "Max score " << maxScore << endl;
		cout << "Current score: " << score << endl;
		SetTextColor(MAGENTA); //purple (magenta)
		cout << map;

		SetTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY); // bright red
		MoveToXY(appleCoords.X, appleCoords.Y);
		cout << apple;
		//SET_DEFAULT_COLOR;

		/* OLD
		newDirection = checkDirection();

		if (((curDirection == LEFT || curDirection == RIGHT) && (newDirection == UP || newDirection == DOWN)) ||
			((curDirection == UP || curDirection == DOWN) && (newDirection == LEFT || newDirection == RIGHT)))
			curDirection = newDirection;

		snakeMove(curDirection, snake);
		*/ 
		//NEW
		snakeMove(changeDirection(curDirection, checkDirection()), snake);

		//draw snake symbols (body) in the X, Y coords
		SetTextColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);  //change color to snake
		for (auto& part : snake) {
			MoveToXY(part.X, part.Y);							//move cursor to snake part coords
			cout << snakeSymb;									//draw the symbol in this place
		} SET_DEFAULT_COLOR;									//return default color



		//moving the snake to one field cell (X,Y), we redefine its head to spec coords creating new element in queue
		//to keep snake lenght const after adding a new element and when it does not changes, we must remove the last one and draw nothing (game field symb)
		if (snake.size() > snakeLen) {  // check if snake lenght corresponds to the size of queue
			COORD tail = snake.back();
			snake.pop_back();
			MoveToXY(tail.X, tail.Y);

			SetTextColor(MAGENTA);
			cout << '.';
			SET_DEFAULT_COLOR;
		}

		// after moving check if the snake collides itself (game over coz it bites itself then)
		if (isBitenItself(snake)) {
			GameOver(_gameOver);
			continue;
		}

		// check if the snake collids edge of game field (if it is then game over)
		if (isKnocked(snake.front())) {
			GameOver(_gameOver);	// выводим тектс об окончании игры и меняем значение _gameOver на true
			continue;				// перебрасываем к началу цикла (проверке условия _gameOver), завершая игру
		}

		// check if an apple has been eaten
		if (isEaten(appleCoords, snake.front())) {

			//PlaySound(TEXT("sound/eat.wav"), NULL, SND_FILENAME | SND_ASYNC);
			++snakeLen; ++score;
			spawnApple(appleCoords, snake);

			// maybe not neadable (!)
			MoveToXY(appleCoords.X, appleCoords.Y);
			SetTextColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
			cout << apple;
		}
		
	}

	if (maxScore < score)
		maxScore = score;
}

char changeDirection(char& _currentDirection, char _newDirection) {

	if (((_currentDirection == LEFT || _currentDirection == RIGHT) && (_newDirection == UP || _newDirection == DOWN)) ||
		((_currentDirection == UP || _currentDirection == DOWN) && (_newDirection == LEFT || _newDirection == RIGHT)))
		_currentDirection = _newDirection;

	return _currentDirection;
}


bool coordsAreCrossing(const COORD&, const deque<COORD>&);

bool isBitenItself(deque<COORD>& _snake) {
	deque<COORD> previousCoord = _snake;
	COORD currentHead = _snake.front();
	previousCoord.pop_front();

	if (coordsAreCrossing(currentHead, previousCoord))
		return true;

	return false;
}

void spawnApple(COORD& _apple, const deque<COORD>& _snake) {
	
	do {
		//_apple.X = (tempCoord = rand() % (WIDTH - Xbound)) == 0 ? tempCoord+1 : tempCoord + 3;
		_apple.X = rand() % (WIDTH - Xbound) + 1;						//rand() % (WIDTH - 3) + 1;
		_apple.Y = rand() % (HEIGHT - 1 - stringsAtALL) + stringsAtALL;	//rand() % (HEIGHT - 1 - 2) + 2;

	} while (coordsAreCrossing(_apple, _snake));
}

bool coordsAreCrossing(const COORD& _obj, const deque<COORD>& _snake) {

	for (const COORD& _coord : _snake)
		if (_coord.X == _obj.X && _coord.Y == _obj.Y)
			return true;
	return false;
}


bool isKnocked(const COORD& coords) {
	
	// min width is 0 (позиция #), max is WIDTH-1 (coz WIDTH is already a symb '\n')
	// min height is 1 (coz the line 0 shows the game score), max height is HEIGHT-1 coz HEIGHT is responsible for sep between the game field and the following messages (empty line)
	if (coords.X <= 0|| coords.X >= WIDTH-2 || coords.Y <= stringsABOVE || coords.Y >= HEIGHT-stringsUNDER)
		return true;
	return false;
}

bool isEaten(COORD& appleCoords, const COORD& snakeCoords) {
	if (snakeCoords.X == appleCoords.X && snakeCoords.Y == appleCoords.Y)
		return true;

	return false;
}

// check winning conditions
bool isWon(unsigned short _score) {

	// the snake must occupy the entire game field, i.e. the score must be equal to the field area without starting snake lenght
	if (_score == inFieldHeight * inFieldWidth - snakeStartLen) 
		return true;
	
	return false;
}

char checkDirection() {
	// check direction using WinAPI and mask 0x8000, then return enum with directions
	// VK_<DIRECTION> - preprocessor macros indicating arrow codes on the keyboard
	if ((GetKeyState('A') | GetKeyState(VK_LEFT)) & 0x8000)
		return LEFT;
	else if ((GetKeyState('D') | GetKeyState(VK_RIGHT)) & 0x8000)
		return RIGHT;
	else if ((GetKeyState('W') | GetKeyState(VK_UP)) & 0x8000)
		return UP;
	else if ((GetKeyState('S') | GetKeyState(VK_DOWN)) & 0x8000)
		return DOWN;

	return NONE;
}

void snakeMove(char dir, deque<COORD>& snakeCoords) {
	// get coords of the current snake head and change depending on the direction
	COORD newHead = snakeCoords.front();

	switch (dir) {
	case LEFT:
		newHead.X--;
		break;
	case RIGHT:
		newHead.X++;
		break;
	case UP:
		newHead.Y--;
		break;
	case DOWN:
		newHead.Y++;
		break;
	default:
		break;
	}
	// insert change coords as new head (move head to new game field cell)
	snakeCoords.push_front(newHead);
}

//void ClearWindow();
void StartGame(bool& _gameOver) {	// start menu with rules, start and exit options, where we define _gameOver value
	
	char key;
	_gameOver = false;

	while (!_gameOver) {
		system("cls");
		cout << "Snake Game" << endl << endl;
		cout << "Press:\ns - start\nr - rules\ne - exit" << endl << endl;
		cout << "Your input: "; cin >> key;

		switch (key) {
		case 's': case'S':
			_gameOver = false;
			return;
		case 'r': case 'R':
			cout << "\nYou play as a snake (symbols '0') that has a base lenght as 2 and moves from start to right." << endl;
			cout << "Snake moves automaticly, but you can choose its direction using WASD on your keyboard" << endl;
			cout << "Your main goal is to occupy the entire playing field eating apples (symbol 'a') that increase your snake's lenght by 1," << endl;
			cout << "but if the snake touches the edges of game field (symbols '#') or bites itself, you lose" << endl << "Good luck!";
			cout << endl << "Press any key to return";
			system("pause>nul");
			break;
		case 'e': case 'E':
			_gameOver = true;
			break;

		default:
			cout << "An unknown key, please write another one" << endl;
			cout << endl << "Press any key to return";
			system("pause>nul");
			break;
		}
	}
}

void GameOver(bool& _gameOver) {

	//score = 110; //debug to check isWon()

	PlaySound(0, 0, 0); //stop prev music
	PlaySound(TEXT("sound/defeat.wav"), NULL, SND_FILENAME | SND_ASYNC);

	MoveToXY(4, HEIGHT/2);
	cout << "Game over!";

	MoveToXY(3, HEIGHT / 2 + 1);
	cout << "Max score: " << score;
	
	// not neccessary to check whether player won during each iteration, we can do it when it ends
	// coz it takes place to final game score after end (when field is filled the snake bite itself or touch the edge of field)
	MoveToXY(0, HEIGHT + 1);
	if (isWon(score))
		cout << "!1!Congratulations!1! \nYour score is the max in-game score!\n\t\tYou have won!1!\n\n";

	_gameOver = true;
}

//void ClearWindow(int x, int y) {
//void ClearWindow() { 
	//MoveToXY(x, y);
	//system("cls");		// clears all console window, so cursor reposition is redundant
//}


//method to clear input that appears after pressing WASD or other keys while playing
void ClearInput() {		// conio.h
	while (_kbhit())	// check if there are not a null-symbols in the line and while they are
		_getch();		// we unload them from the buffer (the value to return is skipped intentionally) and clear the cin 
	
}

int main() {

	bool gameOver;
	char playAgain;


	StartGame(gameOver);
	while (!gameOver) {
	
		system("cls");	// clearing window after method StartGame()
		GameLoop(gameOver); // starting the game loop

		ClearInput(); // clearing player's input after game loop 
		cout << "Play again? (y/n) "; cin >> playAgain;
		if (playAgain == 'y') gameOver = false;
		else gameOver = true;
	}

	//PlaySound(NULL, NULL, 0); //turn off the music when game ends
	// Make cursor position below game window and write a text before game closing
	MoveToXY(0, HEIGHT + 1);
	cout << "Thanks for playing! Bye :)" << endl << "Press any key to leave" << endl;
	system("pause>nul"); // keep the console open untill player press any key

	return 0;
}