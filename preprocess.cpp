#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;

bool intervalsIntersect(int min1, int max1, int min2, int max2) {
  return (min1 >= min2 and min1 <= max2) or (min2 >= min1 and min2 <= max1);
}

struct Rectangle {
  int rmin, cmin, rmax, cmax;
  bool overlaps(const Rectangle& other) const {
    return intervalsIntersect(rmin, rmax, other.rmin, other.rmax) and
           intervalsIntersect(cmin, cmax, other.cmin, other.cmax);
  }
  int area() const {
    return (rmax-rmin+1)*(cmax-cmin+1);
  }
  template<class Archive>
  void serialize(Archive& ar, const unsigned int /* version */) {
    ar & rmin;
    ar & cmin;
    ar & rmax;
    ar & cmax;
  }
};

struct IngredientsCount {
  int tomato, mushroom;
};

int R, C, L, H;
vector<string> pizza;
vector<Rectangle> feasible_rectangles;
vector<vector<int>> incompatibilities;

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
    if (i%100000 == 0) cout << "Progress: " << i << '/' << feasible_rectangles.size() << '\n';
    const Rectangle& rect1 = feasible_rectangles[i];
    for (size_t j = i+1; j < feasible_rectangles.size(); ++j) {
      const Rectangle& rect2 = feasible_rectangles[j];
      if (rect2.rmin > rect1.rmax) break; // huge time saver
      if (rect1.overlaps(rect2)) {
        incompatibilities[i].push_back(j);
        incompatibilities[j].push_back(i);
      }
    }
  }
  cout << "Progress: " << feasible_rectangles.size() << '/' << feasible_rectangles.size() << '\n';
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " test_case (e.g. " << argv[0] << " a_example)\n";
    return 1;
  }

  string in_filename = string("tests/")+argv[1]+".in";
  ifstream in(in_filename);
  if (not in) {
    cout << "Cannot read " << in_filename << '\n';
    return 1;
  }

  string out_filename = string("preprocessed/") + argv[1]+".pre";
  ofstream out(out_filename);
  if (not out) {
    cout << "Cannot write " << out_filename << '\n';
    return 1;
  }

  in >> R >> C >> L >> H;
  pizza.resize(R);
  for (int r = 0; r < R; ++r) in >> pizza[r];
  feasible_rectangles = generateFeasibleRectangles();
  generateIncompatibilities();

  boost::archive::binary_oarchive oa(out);
  oa & feasible_rectangles;
  oa & incompatibilities;
}


