#include <iostream>
#include <vector>
#include <optional>
#include <map>
#include <typeinfo>
#include <unordered_map>
#include "lexer.hpp"

using namespace std;

int main()
{

      class production_def
      {

            
            public:
            enum tuples
            {
                  terminals,
                  non_terminals
            };
            
            struct rules
            {
                  string str;
                  tuples type;
            } token;

            unordered_map<string, vector<vector<rules>>> production;
            // unordered_map<string,unor>;
            
            void production_code(string prod_left, vector<string> prod_right)
            {
                  vector<vector<rules>> set_rules;
                  for (string pk : prod_right)
                  {
                        vector<rules> rule;
                        for (char pk2 : pk)
                        {
                              token.str = pk2;
                              islower(pk2) ? token.type = tuples::terminals : token.type = tuples::non_terminals;
                              rule.push_back(token);
                        }
                        set_rules.push_back(rule);
                  }
                  production[prod_left] = set_rules;
            }

            unordered_map<string, vector<vector<rules>>> prod_return()
            {
                  return production;
            }
            
            unordered_map<string, optional<string>> term_cache;
            
            // optional<string> find_term_in_rule_sequence(vector<rules> &rule_seq)
            // {
            //       for (rules &r : rule_seq)
            //       {
            //             if (r.type == production_def::tuples::terminals)
            //             {
            //                   return r.str;
            //             }
            //             else if (r.type == production_def::tuples::non_terminals)
            //             {
            //                   auto sub = find_term(r.str);
            //                   if (sub.has_value())
            //                         return sub;
            //             }
            //       }
            //       return nullopt;
            // }

            // optional<string> find_term(string str)
            // {
            //       if (term_cache.count(str))
            //             return term_cache[str];

            //       if (production.find(str) == production.end() || production[str].empty())
            //       {
            //             term_cache[str] = nullopt;
            //             return nullopt;
            //       }

            //       for (vector<rules> &rule_seq : production[str])
            //       {
            //             auto term = find_term_in_rule_sequence(rule_seq);
            //             if (term.has_value())
            //             {
            //                   term_cache[str] = term;
            //                   return term;
            //             }
            //       }  

            //       term_cache[str] = nullopt;
            //       return nullopt;
            // }
            optional<string> find_term_from_nonterm(string r){
                  auto it = term_cache.find(r);
                  if (it != term_cache.end()) {
                        return it->second;
                  } else {
                        return nullopt;
                  }
            }


            optional<string> find_term(vector<rules> &rule_seq){
                  for (rules &r : rule_seq){
                        if (r.type == production_def::tuples::terminals){
                              return r.str;
                        }
                        else if (r.type == production_def::tuples::non_terminals){
                              auto sub = find_term_from_nonterm(r.str);
                              if (sub.has_value()){
                                    term_cache[r.str] = sub.value();
                                    return sub.value();
                              }
                        }
                  }
                  return nullopt;
            }
            


            optional<string> follow_term(vector<string> rule_seq){
                  for(auto& rule:rule_seq){
                        
                  }

            }






      };


      vector<string> grammar = {"RBC", "defg"};
      production_def a = production_def();

      a.production_code("S", grammar);
      grammar = {"qst"};
      a.production_code("A", grammar);

      a.production["Q"] = {
          {{"p", production_def::tuples::terminals}, {"A", production_def::tuples::non_terminals}}, // S → a A
          {{"b", production_def::tuples::terminals}}                                                // S → b
      };
      a.production["P"] = {
          {{"X", production_def::tuples::non_terminals}, {"Y", production_def::tuples::non_terminals}}, // S → a A
          {{"Z", production_def::tuples::non_terminals}}                                                // S → b
      };
      grammar = {"Nish", "biSw"};
      a.production_code("D", grammar);

      for (auto& [lhs, rules_list] : a.prod_return())
      {
            cout << lhs << " ::= ";
            for (auto &rule_seq : rules_list)
            {
                  optional<string> term = a.find_term(rule_seq);
                  if (term.has_value())
                  cout << term.value();
                  else
                  cout << "ε";
                  cout << " | ";
            }
            for (auto &rule_seq : rules_list){

            }
            cout << endl;
      }
}