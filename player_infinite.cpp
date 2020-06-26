#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <cassert>
#include <limits.h>

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};


int table[8][8] = {
    {99, -8, 8, 6, 6, 8, -8, 99},
    {-8, -24, -4, -3, -3, -4, -24, -8},
    {8, -4, 7, 4, 4, 7, -4, 8},
    {6, -3, 4, 0, 0, 4, -3, 6},
    {6, -3, 4, 0, 0, 4, -3, 6},
    {8, -4, 7, 4, 4, 7, -4, 8},
    {-8, -24, -4, -3, -3, -4, -24, -8},
    {99, -8, 8, 6, 6, 8, -8, 99}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> board_input, int player, std::vector<Point> next_valid_spots) {
        set(board_input, player, next_valid_spots);
    }
    OthelloBoard(const OthelloBoard &game) {
        disc_count[EMPTY] = game.disc_count[EMPTY];
        disc_count[BLACK] = game.disc_count[BLACK];
        disc_count[WHITE] = game.disc_count[WHITE];
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = game.board[i][j];
            }
        }
        cur_player = game.cur_player;
        next_valid_spots = game.next_valid_spots;
        done = game.done;
        winner = game.winner;
    }
    void set(std::array<std::array<int, SIZE>, SIZE> board_input, int player, std::vector<Point> next_valid_spots_copy) {
        disc_count[EMPTY] = 0;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = board_input[i][j];
                if(board[i][j] == EMPTY)
                    disc_count[EMPTY]++;
                if(board[i][j] == BLACK)
                    disc_count[BLACK]++;
                if(board[i][j] == WHITE)
                    disc_count[WHITE]++;
            }
        }
        cur_player = player;
        next_valid_spots = next_valid_spots_copy;
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y) {
        if (is_spot_valid(Point(x, y))) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    std::string encode_output(bool fail=false) {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8*8-4-disc_count[EMPTY]+1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail) {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        } else if (next_valid_spots.size() > 0) {
            ss << encode_player(cur_player) << "'s turn\n";
        } else {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++) {
            ss << "|";
            for (j = 0; j < SIZE-1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0) {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    std::string encode_state() {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE-1; j++) {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

float evaluationBoard(OthelloBoard game);

float getMaxValue(OthelloBoard game, int depth, float alpha, float beta);

float getMinValue(OthelloBoard game, int depth, float alpha, float beta);

void write_valid_spot(OthelloBoard game_origin, std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    // Keep updating the output until getting killed.
    //while (true) {
        int weight = 0;
        for(int i = 0; i < n_valid_spots; i++) {
            Point p = next_valid_spots[i];
            if(table[p.x][p.y] > weight) {
                weight = table[p.x][p.y];
                fout << p.x << " " << p.y << std::endl;
                fout.flush();
            }
        }

        int defaultdepth = 6;
        if(game_origin.disc_count[0] <= 10) defaultdepth = game_origin.disc_count[0];
        if(n_valid_spots > 16) defaultdepth = 4;
        float bestScore = INT_MIN;
        Point bestMove(-1, -1);
        float alpha = INT_MIN;
        float beta = INT_MAX;
        for(int i = 0; i < n_valid_spots; i++) {
            OthelloBoard game_next(game_origin);
            Point p = next_valid_spots[i];
            game_next.put_disc(p);
        
            float moveScore = getMinValue(game_next, defaultdepth-1, alpha, beta);
            moveScore += table[p.x][p.y];
            if((bestMove.x == -1 && bestMove.y == -1) || bestScore < moveScore) {
                bestScore = moveScore;
                bestMove.x = p.x;
                bestMove.y = p.y;
            }
        }
        // Remember to flush the output to ensure the last action is written to file.
        //std::cout << bestMove.x << " " << bestMove.y << std::endl;
        fout << bestMove.x << " " << bestMove.y << std::endl;
        fout.flush();
    //}
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    OthelloBoard game(board, player, next_valid_spots);
    write_valid_spot(game, fout);
    fin.close();
    fout.close();
    return 0;
}

float evaluationBoard(OthelloBoard game) {
    // need implement
    if(game.disc_count[0] <= 10) {
        return game.disc_count[game.cur_player] - game.disc_count[3-game.cur_player];
    } 
    return (game.next_valid_spots.size() + (game.disc_count[game.cur_player] - game.disc_count[3-game.cur_player])*2.5)*5;
}

float getMaxValue(OthelloBoard game, int depth, float alpha, float beta) {
    if(depth == 0) {return evaluationBoard(game);}

    float best = INT_MIN;

    for(int i = 0; i < game.next_valid_spots.size(); i++) {
        OthelloBoard game_next(game);
        Point p = game_next.next_valid_spots[i];
        game_next.put_disc(p);
        
        float moveScore = getMinValue(game_next, depth-1, alpha, beta);
        moveScore += table[p.x][p.y];
        best = std::max(best, moveScore);
        alpha = std::max(alpha, moveScore);
        if(beta <= alpha) {
            return best;
        }
    }
    return best;
}

float getMinValue(OthelloBoard game, int depth, float alpha, float beta) {
    if(depth == 0) {return evaluationBoard(game);}

    float worst = INT_MAX;

    for(int i = 0; i < game.next_valid_spots.size(); i++) {
        OthelloBoard game_next(game);
        Point p = game_next.next_valid_spots[i];
        game_next.put_disc(p);
        
        float moveScore = getMaxValue(game_next, depth-1, alpha, beta);
        moveScore -= table[p.x][p.y];
        worst = std::min(worst, moveScore);
        beta = std::min(worst, moveScore);
        if(beta <= alpha) {
            return worst;
        }
    }
    return worst;
}
