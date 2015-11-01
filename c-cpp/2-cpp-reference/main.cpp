#include <stdio.h>

class Filter {
public:
  Filter(int& id) : m_id(id) {
    printf("Filter::Filter | m_id=%d\n", m_id);
  }

protected:
   const int& m_id;
};

int main() {
  int id = 18532;
  Filter it = Filter(id);
  Filter& rit = it;

  printf("&it - %x\n", &it);
  printf("&rit - %x\n", &rit);
  printf("rit - %x\n", rit);
}
