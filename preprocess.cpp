#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Rectangle {
  int rmin, cmin, rmax, cmax;
};

struct IngredientsCount {
  int tomato, mushroom;
};

struct Action {
  int type; // 0 -> add, 1->remove
  int slice;
  int score_increment;
};

int R, C, L, H;
vector<string> pizza;
vector<Rectangle> feasible_rectangles;
vector<vector<int>> incompatibilities;

ostream& operator<<(ostream& out, const Rectangle& rect) {
  return out << rect.rmin << ' ' << rect.cmin << ' ' << rect.rmax << ' ' << rect.cmax;
}

int area(const Rectangle& rect) {
  return (rect.rmax-rect.rmin+1)*(rect.cmax-rect.cmin+1);
}

bool intervalsIntersect(int min1, int max1, int min2, int max2) {
  return (min1 >= min2 and min1 <= max2) or (min2 >= min1 and min2 <= max1);
}

bool overlaps(const Rectangle& rect1, const Rectangle& rect2) {
  return intervalsIntersect(rect1.rmin, rect1.rmax, rect2.rmin, rect2.rmax) and
         intervalsIntersect(rect1.cmin, rect1.cmax, rect2.cmin, rect2.cmax);
}

IngredientsCount countIngredientsInRectangle(const Rectangle& rect) {
  IngredientsCount ingredients_count{0,0};
  for (int r = rect.rmin; r <= rect.rmax; ++r) {
    for (int c = rect.cmin; c <= rect.cmax; ++c) {
      if (pizza[r][c] == 'T') ingredients_count.tomato += 1;
      else ingredients_count.mushroom += 1;
    }
  }
  return ingredients_count;
}

void appendRectanglesWithUpperLeftCorner(vector<Rectangle>& rectangles, int rmin, int cmin) {
  for (int rmax = rmin; rmax < min(rmin+H, R); ++rmax) {
    int base = rmax-rmin+1;
    for (int cmax = cmin+max(0,2*L/base-1); cmax < min(cmin+H/base, C); ++cmax) {
      Rectangle rect{rmin, cmin, rmax, cmax};
      IngredientsCount ingredients_count = countIngredientsInRectangle(rect);
      if (ingredients_count.tomato >= L and ingredients_count.mushroom >= L)
        rectangles.push_back(rect);
    }
  }
}

vector<Rectangle> generateFeasibleRectangles() {
  vector<Rectangle> rectangles;
  for (int rmin = 0; rmin < R; ++rmin) {
    for (int cmin = 0; cmin < C; ++cmin) {
      appendRectanglesWithUpperLeftCorner(rectangles, rmin, cmin);
    }
  }
  return rectangles;
}

void generateIncompatibilities() {
  incompatibilities.resize(feasible_rectangles.size());
  for (size_t i = 0; i < feasible_rectangles.size(); ++i) {
    if (i%100000 == 0) cerr << "Progress: " << i << '/' << feasible_rectangles.size() << '\n';
    const Rectangle& rect1 = feasible_rectangles[i];
    for (size_t j = i+1; j < feasible_rectangles.size(); ++j) {
      const Rectangle& rect2 = feasible_rectangles[j];
      if (rect2.rmin > rect1.rmax or rect2.cmin > rect2.cmax) break; // huge time saver
      if (overlaps(rect1, rect2)) {
        incompatibilities[i].push_back(j);
        incompatibilities[j].push_back(i);
      }
    }
  }
  cerr << "Progress: " << feasible_rectangles.size() << '/' << feasible_rectangles.size() << '\n';
}

int main() {
  cin >> R >> C >> L >> H;
  pizza.resize(R);
  for (int r = 0; r < R; ++r) cin >> pizza[r];
  feasible_rectangles = generateFeasibleRectangles();
  generateIncompatibilities();
  cout << feasible_rectangles.size() << '\n';
  for (size_t i = 0; i < feasible_rectangles.size(); ++i) {
    cout << feasible_rectangles[i] << ' ' << area(feasible_rectangles[i]) << '\n';
    cout << incompatibilities[i].size() << '\n';
    for (int incompatibility : incompatibilities[i]) cout << incompatibility << '\n';
  }
}


