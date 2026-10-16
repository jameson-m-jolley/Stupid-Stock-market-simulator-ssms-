#include "core/wallet.hpp"
#include "util/config.hpp"
#include "util/logger.hpp" // added
#include <cmath>
#include <stdexcept>

// this is a class for keeping track of money
//
Wallet::Wallet(int d, int c) {
  // this is a better interface than d and c

  int max_cents = 99;
  if (c > max_cents) {
    // some simple math can be done to change this do work every time
    // we know you cant have more than 100 c becaus that would be a dollar
    int dollars_from_cents = c / (max_cents + 1); // <- this takes the flor
    // if we have 1000 c / 100 = 10
    // if we have 1999 c / 100 = 19 d
    // we then do 1999 c % 100 and get 99
    int remaining_cent = c % (max_cents + 1);

    d += dollars_from_cents;
    c = remaining_cent;
  }

  this->cents = c; // <- do this it works the same and is more readable
  this->dollars = d;

  if (c < 0 || c >= 100) {
    // log before throwing so we capture the bad input
    Logger::get()->error("wallet:[ctor] invalid cents value: {}", c);
    throw std::invalid_argument("not a valid cents");
  }
  cents = c;
  // Logger::get()->info("wallet:[ctor] initialized with ${}.{:02d}", dollars,
  // cents);
}

int Wallet::convertToCents(int d, int c) const {
  if (d < 0) {
    return d * 100 - c;
  }
  return d * 100 + c;
}

std::pair<int, int> Wallet::convertToDollars(int totalCents) const {
  int d = totalCents / 100;
  int rem = totalCents % 100;
  int c = std::abs(rem);

  if (totalCents < 0 && rem != 0) {
    d -= 1;
    c = 100 - c;
  }
  return {d, c};
}

std::pair<int, int> Wallet::add(int d, int c) const {
  int sum = convertToCents(dollars, cents) + convertToCents(d, c);
  return convertToDollars(sum);
}

std::pair<int, int> Wallet::sub(int d, int c) const {
  int diff = convertToCents(dollars, cents) - convertToCents(d, c);
  return convertToDollars(diff);
}

void Wallet::Deposit(int d, int c) {
  auto &conf = config::instance();

  if (conf.debug)
    Logger::get()->info("wallet: deposit request: +${}.{:02d}", d, c);
  std::pair<int, int> result = add(d, c);
  if (conf.debug)
    Logger::get()->info("wallet: deposit applied: ${}.{:02d} -> ${}.{:02d}",
                        dollars, cents, result.first, result.second);
  dollars = result.first;
  cents = result.second;
}

void Wallet::Withdraw(int d, int c) {
  auto &conf = config::instance();
  if (conf.debug)
    Logger::get()->info("wallet: withdraw request: -${}.{:02d}", d, c);
  std::pair<int, int> result = sub(d, c);
  if (conf.debug)
    Logger::get()->info("wallet: withdraw applied: ${}.{:02d} -> ${}.{:02d}",
                        dollars, cents, result.first, result.second);
  dollars = result.first;
  cents = result.second;
}

std::pair<int, int> Wallet::Getwallet() const {
  auto &conf = config::instance();
  if (conf.debug)
    Logger::get()->debug("wallet: Getwallet -> ${}.{:02d}", dollars, cents);
  return {dollars, cents};
}

std::string Wallet::toString() const {
  return "{ \"dollars\": " + std::to_string(dollars) +
         ", \"cents\": " + std::to_string(cents) + " }";
}
