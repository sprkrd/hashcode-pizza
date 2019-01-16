#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <set>
#include <random>
#include <chrono>
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

default_random_engine rng;
int R, C, L, H;
vector<string> pizza;
vector<Rectangle> feasible_rectangles;
vector<vector<int>> incompatibilities;
vector<int> incompatibility_counter;
set<int> slices;
int score = 0;

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

ostream& operator<<(ostream& out, const Rectangle& rect) {
  return out << rect.rmin << ' ' << rect.cmin << ' ' << rect.rmax << ' ' << rect.cmax;
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
}

//void initialSolution() {
  //for (size_t slice = 0; slice < feasible_rectangles.size(); ++slice) {
    ////if (slice%10000 == 0) cerr << "slice " << slice << '/' << feasible_rectangles.size() << '\n';
    //if (not overlapsAnySlice(feasible_rectangles[slice])) {
      //slices.insert(slice);
      //score += area(feasible_rectangles[slice]);
    //}
  //}
//}

Action randomAction() {
  uniform_int_distribution<int> unif(0, feasible_rectangles.size()-1);
  while (true) {
    int slice = unif(rng);
    if (slices.count(slice))
      return {1, slice, -area(feasible_rectangles[slice])};
    else if (incompatibility_counter[slice] == 0)
      return {0, slice, area(feasible_rectangles[slice])};
  }
}

bool accept(double x) {
  if (x > 0) return true;
  double p = exp(x);
  bernoulli_distribution bernoulli(p);
  return bernoulli(rng);
}

void addSlice(int slice) {
  slices.insert(slice);
  for (int other_slice : incompatibilities[slice]) {
    incompatibility_counter[other_slice] += 1;
  }
  score += area(feasible_rectangles[slice]);
}

void removeSlice(int slice) {
  slices.erase(slice);
  for (int other_slice : incompatibilities[slice]) {
    incompatibility_counter[other_slice] -= 1;
  }
  score -= area(feasible_rectangles[slice]);
}

void simulatedAnnealing() {
  const double K_0 = 10;
  const double N = 100000;
  auto maxduration = chrono::seconds(30);
  auto start = chrono::high_resolution_clock::now();
  //initialSolution();
  int n = 0;
  while (chrono::high_resolution_clock::now() - start < maxduration) {
    if (n%10000 == 0)
      cerr << "Iteration " << n << ": score = " << score << '\n';
    double t = K_0/(1+n/N);
    Action action = randomAction();
    if (accept(action.score_increment/t)) {
      if (action.type == 0) addSlice(action.slice);
      else removeSlice(action.slice);
    }
    ++n;
  }
}

int main() {
  rng.seed(chrono::system_clock::now().time_since_epoch().count());

  cin >> R >> C >> L >> H;
  pizza.resize(R);
  for (int r = 0; r < R; ++r) cin >> pizza[r];
  feasible_rectangles = generateFeasibleRectangles();
  generateIncompatibilities();
  incompatibility_counter.resize(feasible_rectangles.size(), 0);
  simulatedAnnealing();
  cerr << "Simulated annealing has given a solution with score " << score << '\n';
  cout << slices.size() << endl;
  for (int slice : slices)
    cout << feasible_rectangles[slice] << endl;

}


