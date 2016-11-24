#include <stdio.h>

class Base {
public:
  Base(int id) {
    printf("Base's constructor; id=%d\n", id);
  }
};

class Filter: public Base {
public:
  Filter(int id): Base(id), m_index(42), m_const_index(142) {
    printf("Filter's constructor\n");
  }

  void Call(void) {
    printf("Filter->Call | m_index: %d | m_const_index: %d \n", m_index, m_const_index);
  }

private:
  int m_index;
  const int m_const_index;
};

int main() {
  Filter* filter = new Filter(1);
  filter->Call();
}
