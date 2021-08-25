#include "dd_exchange.hpp"

#include <string>

using namespace sylvan;

SddManager* Sdd::Manager = sdd_manager_create(2*((7*7)-(4*4)), 0);

VOID_TASK_DECL_4(printboard, void*, BDDVAR*, uint8_t*, int);
#define printboard TASK(printboard)

enum Square {
    NN = 0, // No square
    OO = 1, // Peg on square (on initial board)
    EE = 2, // Empty square  (on initial board)
};

#define N 7
Square BOARD[N][N] = {
    {NN,NN,OO,OO,OO,NN,NN,},
    {NN,NN,OO,OO,OO,NN,NN,},
    {OO,OO,OO,OO,OO,OO,OO,},
    {OO,OO,OO,EE,OO,OO,OO,},
    {OO,OO,OO,OO,OO,OO,OO,},
    {NN,NN,OO,OO,OO,NN,NN,},
    {NN,NN,OO,OO,OO,NN,NN,},
};

struct PartialRelation
{  
  dd MoveRelation;
  ddset RelationVars;
};

class PegSolitaire
{
  public:
    PegSolitaire();
    void CreateInitialBoard();
    void CreateWinningBoards();
    void CreateMoveRelation();
    void PrintBoards(dd &D, std::string name);
    
    dd InitialBoard;
    dd WinningBoards;
    dd MoveRelation;
    ddvar Coor2Var[N][N];
    ddset SourceVars;
    ddset TargetVars;
    ddset RelationVars;
    std::vector<ddset> Triples;
    std::vector<PartialRelation> Relations;
};

PegSolitaire::PegSolitaire()
{
  ddvar var;
  if (dd_choice) 
    var = 0;
  else          
    var = 1;

  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      if (BOARD[i][j] == NN) 
        continue;

      ddvar source = var++;  
      ddvar target = var++;

      Coor2Var[i][j] = source;
      
      SourceVars.add(source);  
      TargetVars.add(target);
    }
  }

  RelationVars.add(SourceVars);
  RelationVars.add(TargetVars);

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        continue;
        
      if (j+2 < N && BOARD[i][j+1] != NN && BOARD[i][j+2] != NN) 
      {
        ddset Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i][j+1]);
        Triple.add(Coor2Var[i][j+2]);
        Triples.push_back(Triple);
      }

      if (i+2 < N && BOARD[i+1][j] != NN && BOARD[i+2][j] != NN) 
      {
        ddset Triple;
        Triple.add(Coor2Var[i][j]);
        Triple.add(Coor2Var[i+1][j]);
        Triple.add(Coor2Var[i+2][j]);
        Triples.push_back(Triple);
      }    
    }
  }
}

void 
PegSolitaire::CreateInitialBoard()
{
  InitialBoard = DecisionDiagram::ddOne();

  for (int i = 0; i < N; i++) 
  {
    for (int j = 0; j < N; j++) 
    {
      if (BOARD[i][j] == NN)
        continue;
      ddvar square = Coor2Var[i][j];
      if (BOARD[i][j] == EE)
        InitialBoard = InitialBoard & !DecisionDiagram::create(square);
      else 
        InitialBoard = InitialBoard & DecisionDiagram::create(square);
    }
  }
}

void PegSolitaire::CreateWinningBoards() 
{
  WinningBoards = DecisionDiagram::ddZero(); 
  for (ddvar square : SourceVars.toVector()) 
  {
    dd Board = DecisionDiagram::ddOne();  
    for (ddvar square2 : SourceVars.toVector()) 
    {
      if (square2 == square) 
        Board &= DecisionDiagram::create(square);
      else 
        Board &= !DecisionDiagram::create(square2);
    }
    WinningBoards = WinningBoards | Board;
  }
}

void PegSolitaire::CreateMoveRelation()
{
  for (ddset &Triple : Triples)
  {
    std::vector<ddvar> V = Triple.toVector();
    PartialRelation Partial;
    for (ddvar var : V)
    {
      Partial.RelationVars.add(var);
      Partial.RelationVars.add(var+1);
    }   
    Partial.MoveRelation = DecisionDiagram::ddZero();
    
    dd move1 =  DecisionDiagram::create(V[0]) & !DecisionDiagram::create(V[0]+1) &
                DecisionDiagram::create(V[1]) & !DecisionDiagram::create(V[1]+1) &
               !DecisionDiagram::create(V[2]) &  DecisionDiagram::create(V[2]+1);
               
    Partial.MoveRelation |= move1;
    
    dd move2 = (!DecisionDiagram::create(V[0])) &  DecisionDiagram::create(V[0]+1) &
                 DecisionDiagram::create(V[1])  & !DecisionDiagram::create(V[1]+1) &
                 DecisionDiagram::create(V[2])  & !DecisionDiagram::create(V[2]+1);
                
    Partial.MoveRelation |= move2;
    Relations.push_back(Partial);
  }
  size_t total_nodes = 0;
  MoveRelation = DecisionDiagram::ddZero();
  for (PartialRelation& Partial : Relations)
  {
    dd Rel = Partial.MoveRelation;
    for (ddvar var : SourceVars.toVector())
    {
      if (Partial.RelationVars.contains(var))
        continue;
      Rel &= DecisionDiagram::create(var).Xnor(DecisionDiagram::create(var+1));
    }
    MoveRelation |= Rel;
    total_nodes += Partial.MoveRelation.NodeCount();
  }
  printf("Partial relations have a total of %ld nodes\n", total_nodes);
}

void printboardsSDD(int *e)
{
  printf("--------------------\n");
  int teller = 0;
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (BOARD[i][j] == NN)
        printf(" , ");
      else
        printf("%d, ",e[teller++]);
    }
    printf("\n");
  }
  printf("--------------------\n");
}

void 
PegSolitaire::PrintBoards(dd &D, std::string name) 
{
  printf("Printing boards: %s \n", name.c_str());
  //sylvan_enum(D.GetDD(), SourceVars.GetDD(), printboard, this);
  SddExtra::set_enum(D.GetDD(), Sdd::Manager, printboardsSDD);
}

VOID_TASK_IMPL_4(printboard, void*, ctx, BDDVAR*, VA, uint8_t*, values, int, count) {
    PegSolitaire *Game = (PegSolitaire *) ctx;
    printf("--------------------\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (BOARD[i][j] == NN) {
                printf(" , ");
            } else {
                BDDVAR var = Game->Coor2Var[i][j];
                for (int k = 0; k < count; k++) {
                    if (var == VA[k]) {
                        printf("%s, ", (values[k] ? "1" : "0"));
                        break;
                    }
                }
            }
        }
        printf("\n");
    }
    printf("--------------------\n");
}

int main()
{
  PegSolitaire game;
  game.CreateInitialBoard();
  game.CreateMoveRelation();
  game.PrintBoards(game.InitialBoard, "InitialBoard");
  //game.InitialBoard = game.InitialBoard.RelNext(game.MoveRelation, game.RelationVars);
  //game.PrintBoards(game.InitialBoard, "Next");
}
