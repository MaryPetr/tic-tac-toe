#include <cstddef>
#include <cstdlib>//for exit()
#include <iostream>//for dialog
#include <string>//for dialog
#include <cstring>//for dialog
#include<math.h>
#include <fstream>
#include <string>
#include < cstdio >

//Реализация вывода индексов поля в консоль
//Сохранение игры в фа1йл 
// строчки 473-581

// =================================== Utils

void successFinishApplication() {
	exit(EXIT_SUCCESS);
}

void failureFinishApplication() {
	exit(EXIT_FAILURE);
}

//TODO: Show conditional compilation here
void check(const bool condition, const char* errorToPrint) {
	if (!condition) {
		std::cout << errorToPrint << std::endl;
		failureFinishApplication();
	}
}

// =================================== Field

enum class ECellValue : uint8_t
{
	Empty,
	X,
	O
};

struct Field
{
	ECellValue* bufferPointer = nullptr;
	int32_t width = 0;
	int32_t height = 0;
};

struct CellPosition
{
	int32_t x = 0;
	int32_t y = 0;
};

bool isFieldInitialized(Field& field) {
	return (field.bufferPointer != nullptr);
}

void initField(Field& field, const int32_t width, const int32_t height) {
	check(!isFieldInitialized(field), "initField: Field is already initialized");

	const int32_t fieldSize = width * height;
	field.bufferPointer = new ECellValue[fieldSize];
	field.width = width;
	field.height = height;
}

bool isValidPosition(const Field& field, const CellPosition& cellPosition) {
	return (cellPosition.x >= 0 && cellPosition.x < field.width) &&
		(cellPosition.y >= 0 && cellPosition.y < field.height);
}

ECellValue& getFieldCellRef(Field& field, const CellPosition& cellPosition) {
	check(isValidPosition(field, cellPosition), "getFieldCellRef: Invalid coordinate provided");

	const int32_t cellOffset = field.width * cellPosition.y + cellPosition.x;
	return *(field.bufferPointer + cellOffset);
}

//TODO: Show const cast here
ECellValue getFieldCellRef(const Field& field, const CellPosition& cellPosition) {
	check(isValidPosition(field, cellPosition), "getFieldCellRef: Invalid coordinate provided");

	const int32_t cellOffset = field.width * cellPosition.y + cellPosition.x;
	return *(field.bufferPointer + cellOffset);
}

void resetField(Field& field) {
	for (int32_t y = 0; y < field.height; ++y) {
		for (int32_t x = 0; x < field.width; ++x) {
			ECellValue& cellRef = getFieldCellRef(field, CellPosition{ x, y });
			cellRef = ECellValue::Empty;
		}
	}
}

// =================================== Player

enum class EPlayer : uint8_t
{
	X,
	O
};

// =================================== Game


struct GameState
{
	Field field;
	EPlayer currentPlayer = EPlayer::X;

	enum class EStatus : uint8_t
	{
		NotInitialized,
		Playing,
		FinishedWinnedByCurrentPlayer,
		FinishedInDraw
	};
	EStatus status = EStatus::NotInitialized;
	int32_t filledCellsNumForFastDrawTesting = 0;
};

void resetGame(GameState& gameState,
	const EPlayer firstPlayer = EPlayer::X)
{
	check(gameState.status != GameState::EStatus::NotInitialized,
		"initGame: Invalid game status");

	resetField(gameState.field);
	gameState.currentPlayer = firstPlayer;
	gameState.status = GameState::EStatus::Playing;
	gameState.filledCellsNumForFastDrawTesting = 0;
}

void initGame(GameState& gameState,
	const int32_t width,
	const int32_t height,
	const EPlayer firstPlayer = EPlayer::X)
{
	check(gameState.status == GameState::EStatus::NotInitialized,
		"initGame: Invalid game status");

	initField(gameState.field, width, height);
	gameState.status = GameState::EStatus::Playing; //workaround for check passing

	resetGame(gameState, firstPlayer);
}

bool isValidPositionForTurn(const GameState& gameState, const CellPosition& position) {
	return isValidPosition(gameState.field, position) &&
		getFieldCellRef(gameState.field, position) == ECellValue::Empty;
}

ECellValue getCellValueForPlayer(const EPlayer player) {
	switch (player) {
	case EPlayer::X:  return ECellValue::X;
	case EPlayer::O:  return ECellValue::O;
	default:          break;
	}

	check(false, "getCellValueForPlayer: Unreachable code reached");
	return ECellValue::Empty;
}

EPlayer getOppositePlayer(const EPlayer player) {
	switch (player) {
	case EPlayer::X:  return EPlayer::O;
	case EPlayer::O:  return EPlayer::X;
	default:          break;
	}

	check(false, "getOppositePlayer: Unreachable code reached");
	return EPlayer::X;
}


struct WinCheckingStep
{
	int8_t x = 0;
	int8_t y = 0;
};

const int8_t winLineSize = 5;

bool lineWinCheck(GameState& gameState,
	const CellPosition& cellPosition,
	const ECellValue cellValue,
	const WinCheckingStep& step)
{
	int8_t valuesInLine = 1;

	CellPosition currentCell;

	currentCell.x = cellPosition.x + step.x;
	currentCell.y = cellPosition.y + step.y;
	while (isValidPosition(gameState.field, currentCell) &&
		getFieldCellRef(gameState.field, currentCell) == cellValue)
	{
		currentCell.x += step.x;
		currentCell.y += step.y;
		++valuesInLine;

		if (valuesInLine == winLineSize)
			return true;
	}

	currentCell.x = cellPosition.x - step.x;
	currentCell.y = cellPosition.y - step.y;
	while (isValidPosition(gameState.field, currentCell) &&
		getFieldCellRef(gameState.field, currentCell) == cellValue)
	{
		currentCell.x -= step.x;
		currentCell.y -= step.y;
		++valuesInLine;

		if (valuesInLine == winLineSize)
			return true;
	}

	return false;
}

void makeTurn(GameState& gameState, const CellPosition& cellPosition) {
	check(gameState.status == GameState::EStatus::Playing &&
		isValidPositionForTurn(gameState, cellPosition), "makeTurn: Invalid turn provided");

	//Change cell
	const ECellValue cellValueForCurrentPlayer = getCellValueForPlayer(gameState.currentPlayer);
	getFieldCellRef(gameState.field, cellPosition) = cellValueForCurrentPlayer;
	++gameState.filledCellsNumForFastDrawTesting;

	//Check winning
	const bool isCurrentTurnWinning =
		lineWinCheck(gameState, cellPosition, cellValueForCurrentPlayer, WinCheckingStep{ 1, 0 }) ||
		lineWinCheck(gameState, cellPosition, cellValueForCurrentPlayer, WinCheckingStep{ 0, 1 }) ||
		lineWinCheck(gameState, cellPosition, cellValueForCurrentPlayer, WinCheckingStep{ 1, 1 }) ||
		lineWinCheck(gameState, cellPosition, cellValueForCurrentPlayer, WinCheckingStep{ 1, -1 });

	if (isCurrentTurnWinning) {
		gameState.status = GameState::EStatus::FinishedWinnedByCurrentPlayer;
	}
	else if (gameState.filledCellsNumForFastDrawTesting ==
		gameState.field.width * gameState.field.height)
	{
		gameState.status = GameState::EStatus::FinishedInDraw;
	}
	else {
		gameState.currentPlayer = getOppositePlayer(gameState.currentPlayer);
	}
}

// ==================================== Input

namespace Input
{
	// -------------------- Turn position dialog

	struct TurnPositonDialogResult
	{
		enum class EOption : uint8_t
		{
			TurnSelected,
			MenuOpenRequest
		};
		EOption option;

		CellPosition positionIfProvided;
	};

	TurnPositonDialogResult performTurnPositionDialog(const GameState& gameState)
	{
		static const char* const dialogStartingCaption = "Please, enter turn coordinates or request game menu (\"M\")";
		static const char* const captionForXCoordinate = "x: ";
		static const char* const captionForYCoordinate = "y: ";
		static const char* const captionForInvalidPositionProvided = "Invalid position provided. Try again, please...";

		static const char* const menuCommandString = "M";

		std::cout << dialogStartingCaption << std::endl;

		std::string inputString;

		do {
			CellPosition turnPosition;

			//Try fill X position
			std::cout << captionForXCoordinate;
			std::getline(std::cin, inputString);
			if (strcmp(inputString.c_str(), menuCommandString) == 0)
				return TurnPositonDialogResult{ TurnPositonDialogResult::EOption::MenuOpenRequest };

			turnPosition.x = atoi(inputString.c_str());

			//Try fill Y position
			std::cout << captionForYCoordinate;
			std::getline(std::cin, inputString);
			if (strcmp(inputString.c_str(), menuCommandString) == 0)
				return TurnPositonDialogResult{ TurnPositonDialogResult::EOption::MenuOpenRequest };

			turnPosition.y = atoi(inputString.c_str());

			//Check position
			if (isValidPositionForTurn(gameState, turnPosition))
				return { TurnPositonDialogResult::EOption::TurnSelected, turnPosition };

			std::cout << captionForInvalidPositionProvided << std::endl;

		} while (true);

		check(false, "performTurnPositionDialog: Unreachable code reached");
		return TurnPositonDialogResult{ };
	}

	// -------------------- Menu dialog

	struct GameMenuDialogResult
	{
		enum class EOption : uint8_t
		{
			MenuExitRequested,
			GameRestartRequested,
			GameQuitRequested,
			GameSaveRequested
		};
		EOption option;
	};

	GameMenuDialogResult performMenuDialog() {
		static const char* const dialogStartingCaption = "You are in game menu. Please, select option:\n"
			"(E) to exit from menu\n"
			"(R) to restart game\n"
			"(Q) to quit game\n"
			"(S) to save game\n";
		static const char* const captionForInvalidOptionProvided = "Invalid option provided. Try again, please...";

		static const char* const menuExitCommandString = "E";
		static const char* const gameRestartCommandString = "R";
		static const char* const gameQuitCommandString = "Q";
		static const char* const gameSaveCommandString = "S";

		std::cout << dialogStartingCaption << std::endl;

		std::string inputString;

		do {
			std::getline(std::cin, inputString);
			if (strcmp(inputString.c_str(), menuExitCommandString) == 0)
				return GameMenuDialogResult{ GameMenuDialogResult::EOption::MenuExitRequested };
			else if (strcmp(inputString.c_str(), gameRestartCommandString) == 0)
				return GameMenuDialogResult{ GameMenuDialogResult::EOption::GameRestartRequested };
			else if (strcmp(inputString.c_str(), gameQuitCommandString) == 0)
				return GameMenuDialogResult{ GameMenuDialogResult::EOption::GameQuitRequested };
			else if (strcmp(inputString.c_str(), gameSaveCommandString) == 0)
				return GameMenuDialogResult{ GameMenuDialogResult::EOption::GameSaveRequested };

			std::cout << captionForInvalidOptionProvided << std::endl;
		} while (true);

		check(false, "performMenuDialog: Unreachable code reached");
		return GameMenuDialogResult{ };
	}

	// -------------------- Turn dialog

	struct TurnDialogResult
	{
		enum class EOption : uint8_t
		{
			TurnSelected,
			GameRestartRequested,
			GameQuitRequested,
			GameSaveRequested
		};
		EOption option;

		CellPosition turnPositionIfProvided;
	};

	TurnDialogResult performTurnDialog(const GameState& gameState) {
		do
		{
			const TurnPositonDialogResult turnPositonDialogResult =
				performTurnPositionDialog(gameState);

			switch (turnPositonDialogResult.option) {
				case TurnPositonDialogResult::EOption::TurnSelected:
				return TurnDialogResult{
					TurnDialogResult::EOption::TurnSelected,
					turnPositonDialogResult.positionIfProvided };

				case TurnPositonDialogResult::EOption::MenuOpenRequest: {
				const GameMenuDialogResult gameMenuDialogResult =
					performMenuDialog();

			switch (gameMenuDialogResult.option) {
				case GameMenuDialogResult::EOption::MenuExitRequested:
					break;
				case GameMenuDialogResult::EOption::GameRestartRequested:
					return TurnDialogResult{ TurnDialogResult::EOption::GameRestartRequested };
				case GameMenuDialogResult::EOption::GameQuitRequested:
					return TurnDialogResult{ TurnDialogResult::EOption::GameQuitRequested };

				case GameMenuDialogResult::EOption::GameSaveRequested:
					return TurnDialogResult{ TurnDialogResult::EOption::GameSaveRequested };
				}
				

				break;
			}

			default:
				check(false, "performTurnDialog: Unsupported option reached");
				break;
			}

		} while (true);

		check(false, "performTurnDialog: Unreachable code reached");
		return TurnDialogResult{ };
	}

	void debugPrintTurnDialogResult(const TurnDialogResult& turnDialogResult) {
		switch (turnDialogResult.option) {
		case TurnDialogResult::EOption::TurnSelected: {
			const CellPosition& position = turnDialogResult.turnPositionIfProvided;
			std::cout
				<< "DEBUG: TurnSelected " << "[" << position.x << ", " << position.y << "]"
				<< std::endl;
			break;
		}

		case TurnDialogResult::EOption::GameRestartRequested:
			std::cout
				<< "DEBUG: GameRestartRequested "
				<< std::endl;
			break;

		case TurnDialogResult::EOption::GameQuitRequested:
			std::cout
				<< "DEBUG: GameQuitRequested "
				<< std::endl;
			break;

		default:
			check(false, "debugPrintTurnDialogResult: Unsupported option reached");
			break;
		}
	}
}

// ==================================== Render

namespace Render
{
	namespace RenderSymbols
	{
		const char X = 'X';
		const char O = 'O';
		const char Empty = '.';
	}

	char getRenderSymbolForCellValue(const ECellValue cellValue) {
		switch (cellValue) {
		case ECellValue::Empty:		return RenderSymbols::Empty;
		case ECellValue::X:			return RenderSymbols::X;
		case ECellValue::O:			return RenderSymbols::O;

		default:
			check(false, "getRenderSymbolForCellValue: Invalid cell value");
			return '\0';
		}
	}
	//Вывод индексов поля в консоль
	int numDigits(int32_t num)
	{
		int DigitCount = 0;
		while (num > 0) {
			DigitCount++;
			num /= 10;
		}
		return DigitCount;
	}

	void outputSpace(int32_t index, int32_t maxvaluesize){
		 if (index == 0) {//Костыль надо бы исправить
			for (int32_t i = 0; i <= maxvaluesize - 1; i++) {

				std::cout << " ";
			}					
			
			return;
		}
		for (int32_t i = 0; i <= maxvaluesize - numDigits(index); i++) {
			
			std::cout << " ";
		}  
		
	}
	
	//Запись игры в файл
	void renderFieldToFile(const Field& field) {
	
		std::ofstream fileToWriteTo("file1.txt", std::ios::app);
		// индексы по x;
		int32_t maxvaluesizeY = numDigits(field.height);
		int32_t maxvaluesizeX = numDigits(field.width);
		int32_t indexX = 0;
		int32_t indexY = 0;
		std::cout << "   ";
		while (indexX < field.width) {
			fileToWriteTo << indexX << "  ";
			
			indexX++;

		}
		fileToWriteTo << '\n';
		// -----------------

		for (int32_t y = 0; y < field.height; ++y) {

			//индексы по y;
			fileToWriteTo << indexY << "  ";
			indexY++;
			// ----------------
			

			for (int32_t x = 0; x < field.width; ++x) {
				const ECellValue cellValue = getFieldCellRef(field, CellPosition{ x, y });
				fileToWriteTo << getRenderSymbolForCellValue(cellValue) << "  ";

			}
		
			fileToWriteTo << '\n';
			
		}
		fileToWriteTo.close();
	}

	
	
	void renderGameToFile(const GameState& gameState) {
		renderFieldToFile(gameState.field);
	}
	void renderFieldToConsole(const Field& field) {


		// индексы по x;
		int32_t maxvaluesizeY = numDigits(field.height);
		int32_t maxvaluesizeX = numDigits(field.width);
		int32_t indexX = 0;
		int32_t indexY = 0;
		std::cout << "   ";
		while (indexX < field.width) {
			std::cout << indexX;
			outputSpace(indexX, maxvaluesizeX);
			indexX++;
			
		}
		std::cout << '\n';
		// -----------------

		for (int32_t y = 0; y < field.height; ++y) {

			//индексы по y;
			std::cout << indexY;
			outputSpace(indexY, maxvaluesizeY);
			indexY++;
			// ----------------
			

			for (int32_t x = 0; x < field.width; ++x) {
				const ECellValue cellValue = getFieldCellRef(field, CellPosition{ x, y });
				
				std::cout << getRenderSymbolForCellValue(cellValue) << "  ";
		
			}
			std::cout << std::endl;

		}
		
	}

	void renderGameToConsole(const GameState& gameState) {
		renderFieldToConsole(gameState.field);
	}
}

// ====================================

int main() {
	GameState gameState;
	std::ofstream fileToWriteTo("file1.txt", std::ios::trunc);
	fileToWriteTo.close();		


	initGame(gameState, 10, 10, EPlayer::X);

	do
	{
		std::cout << "_______________" << std::endl;

		Render::renderGameToConsole(gameState);
		Render::renderGameToFile(gameState);

		//renderGameToConsole(gameState);
		const Input::TurnDialogResult turnDialogResult = Input::performTurnDialog(gameState);

		switch (turnDialogResult.option) {
		case Input::TurnDialogResult::EOption::TurnSelected:
			makeTurn(gameState, turnDialogResult.turnPositionIfProvided);

			if (gameState.status == GameState::EStatus::FinishedWinnedByCurrentPlayer)
				std::cout << "DEBUG: WinGame" << std::endl;

			break;

		case Input::TurnDialogResult::EOption::GameRestartRequested:
			resetGame(gameState, EPlayer::X);
			break;

		case Input::TurnDialogResult::EOption::GameQuitRequested:
			successFinishApplication();
			break;


		default:
			check(false, "main: Unsupported option reached");
			break;
		}
	} while (true);

	return 0;
}
