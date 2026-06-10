#pragma once
#include <iostream>
#include <map>
#include "Client.hpp"
#include <set>
class chanel{

    private :
        std::string name_chanel ;
        std::string pass_chanel ;
        bool invitation;
        bool t;
        bool o;
        int  limits;
        bool password; // pass 

        std::map <int , Client *> all_Cchanel;
        std::map <int , Client *> op_chanel; // ga3 les admin dyal had chanel ;
        std::set <std::string> wait_list ;
    public :
        chanel();
        void set_pass(std::string & str) ;
        void set_name(std::string & str) ;
        void add_CTOchanel(Client & client) ;

        

        std::map <int , Client *>& get_opChanel();
        std::map <int , Client *>& get_All_Cchanel();
        std::set <std::string> &get_list() ; 

        bool get_password() const ;
        bool get_invetation() const ;
        int  get_limitchanell() const ;

        bool is_member(int fd ) ;
        bool is_operator(int fd ) ;

        std::string get_passwordChanel() const ;
        std::string get_namechanel() const ;
};