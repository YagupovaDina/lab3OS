#include "List.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <atomic>
#include <string>
#include "stm/stm.h"
#include "stm/context.h"

#define NUM_ELEMENTS 10000

stm::Context cxt;
stm::TVar<List<int>> global_list;

void push_to_list(int c)
{
  auto f_push_to_list = [&](List<int> l)
  {
    return stm::writeTVar(global_list, l.inserted(c));
  };
  auto push_transaction = stm::withTVar<List<int>, stm::Unit>(global_list, f_push_to_list);
  stm::atomically(cxt, push_transaction);
}

int pop_from_list()
{
  auto f_pop_from_list = [](List<int> l)
  {
    if(l.isEmpty())
      return stm::retry<int>();
    else
    {
      int h = l.front();
      auto write_tvar = stm::writeTVar(global_list, l.popped_front());
      auto f_return_h = [h](stm::Unit)
      {
        return stm::pure(h);
      };
      return stm::bind<stm::Unit, int>(write_tvar, f_return_h);
    }
  };
  auto pop_transaction = stm::withTVar<List<int>, int>(global_list, f_pop_from_list);
  return stm::atomically(cxt, pop_transaction);
}

void print_list()
{
  auto read_list = stm::readTVar(global_list);
  auto list = stm::atomically(cxt, read_list);
  print(list);
}

bool is_list_empty()
{
  auto f_list_empty = [](List<int> l)
  {
    return stm::pure(l.isEmpty());
  };
  auto check_transaction = stm::withTVar<List<int>, bool>(global_list,  f_list_empty);
  return stm::atomically(cxt, check_transaction);
}

void fill_list()
{
  for(int i = 0; i < NUM_ELEMENTS; i++)
  {
    push_to_list(i);
  }
}

void merge_list(stm::TVar<List<int>> list1, stm::TVar<List<int>> list2, stm::TVar<List<int>> result_list){
 	auto read_list1 = stm::readTVar(list1);
	auto read_list2 = stm::readTVar(list2);
  	auto stm_list1 = stm::atomically(cxt, read_list1);
	auto stm_list2 = stm::atomically(cxt, read_list2);

	auto f_merge_to_list = [&](List<int> l)
	{
		l = merged(stm_list1, stm_list2);
		return stm::writeTVar(result_list, l);
	};
	auto push_transaction = stm::withTVar<List<int>, stm::Unit>(result_list, f_merge_to_list);
	stm::atomically(cxt, push_transaction);
}

void read_from_list(int* acc)
{
  for(int i = 0; i < NUM_ELEMENTS; i++)
  {
    *acc += pop_from_list();
  }
}

int main()
{
    int acc1 = 0;
    int acc2 = 0;
    int acc3 = 0;

    global_list = stm::newTVarIO<List<int>>(cxt, List<int>());
    std::thread fl1(fill_list);
    std::thread fl2(fill_list);
    std::thread fl3(fill_list);
 std::thread fl4(print_list);
    std::thread rd1(read_from_list, &acc1);
    std::thread rd2(read_from_list, &acc2);
    std::thread rd3(read_from_list, &acc3);


    fl1.join();
    fl2.join();
    fl3.join();
fl4.join();
    rd1.join();
    rd2.join();
    rd3.join();

    int acc = acc1 + acc2 + acc3;

    std::cout << "acc1 = " << acc1 << std::endl;
    std::cout << "acc2 = " << acc2 << std::endl;
    std::cout << "acc3 = " << acc3 << std::endl;
    std::cout << "acc = " << acc << std::endl;

    return 0;

}

