#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <set>
#include <random>
#include <chrono>

#include <boost/archive/binary_iarchive.hpp>
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

struct Action {
  int type; // 0 -> add, 1->remove
  int slice;
};

default_random_engine rng;
vector<Rectangle> feasible_rectangles;
vector<vector<int>> incompatibilities;
vector<int> incompatibility_counter;
set<int> slices;
int score = 0;

ostream& operator<<(ostream& out, const Rectangle& rect) {
  return out << rect.rmin << ' ' << rect.cmin << ' ' << rect.rmax << ' ' << rect.cmax;
}

Action randomAction() {
  uniform_int_distribution<int> unif(0, feasible_rectangles.size()-1);
  while (true) {
    int slice = unif(rng);
    if (slices.count(slice))
      return {1, slice};
    else if (incompatibility_counter[slice] == 0)
      return {0, slice};
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
  score += feasible_rectangles[slice].area();
}

void removeSlice(int slice) {
  slices.erase(slice);
  for (int other_slice : incompatibilities[slice]) {
    incompatibility_counter[other_slice] -= 1;
  }
  score -= feasible_rectangles[slice].area();
}

void initialSolution() {
  for (size_t slice = 0; slice < feasible_rectangles.size(); ++slice) {
    if (incompatibility_counter[slice] == 0) addSlice(slice);
  }
}

void simulatedAnnealing(int timeout, double K_0, double N) {
  set<int> best_solution = slices;
  int max_score = score;
  auto maxduration = chrono::seconds(timeout);
  auto start = chrono::high_resolution_clock::now();
  int n = 0;
  while (chrono::high_resolution_clock::now() - start < maxduration) {
    if (n%10000 == 0)
      cout << "Iteration " << n << ": score = " << score << '\n';
    double t = K_0/(1+n/N);
    Action action = randomAction();
    int sign = action.type == 0? 1 : -1;
    int score_increment = sign*feasible_rectangles[action.slice].area();
    if (accept(score_increment/t)) {
      if (action.type == 0) addSlice(action.slice);
      else removeSlice(action.slice);
      if (score > max_score) {
        best_solution = slices;
        max_score = score;
      }
    }
    ++n;
  }
  slices = best_solution;
  score = max_score;
}

int main(int argc, char* argv[]) {
  rng.seed(chrono::system_clock::now().time_since_epoch().count());

  if (argc != 5) {
    cout << "Usage: " << argv[0] << " test_case timeout K_0 N (e.g. " << argv[0] << " a_example 60 10 10000)\n";
    return 1;
  }

  int timeout = stoi(argv[2]);
  double K_0 = stod(argv[3]);
  double N = stod(argv[4]);

  string in_filename = string("preprocessed/")+argv[1]+".pre";
  ifstream in(in_filename);
  if (not in) {
    cout << "Cannot read " << in_filename << '\n';
    return 1;
  }

  string out_filename = string("output/") + argv[1]+".out";
  ofstream out(out_filename);
  if (not out) {
    cout << "Cannot write " << out_filename << '\n';
    return 1;
  }

  boost::archive::binary_iarchive ia(in);
  ia & feasible_rectangles;
  ia & incompatibilities;

  incompatibility_counter.resize(feasible_rectangles.size(), 0);

  initialSolution();
  simulatedAnnealing(timeout, K_0, N);

  cout << "Simulated annealing has given a solution with score " << score << '\n';
  out << slices.size() << endl;
  for (int slice : slices)
    out << feasible_rectangles[slice] << endl;

}


