#ifndef WALLET_HPP
#define WALLET_HPP

#include <string>
#include <utility>

class Wallet {
private:
  int dollars;
  int cents;

public:
  Wallet(int d = 0, int c = 0);

  int convertToCents(int d, int c) const;
  std::pair<int, int> convertToDollars(int totalCents) const;

  std::pair<int, int> add(int d, int c) const;
  std::pair<int, int> sub(int d, int c) const;

  void Deposit(int d, int c);
  void Withdraw(int d, int c);

  std::pair<int, int> Getwallet() const;
  std::string toString() const;
};

#endif