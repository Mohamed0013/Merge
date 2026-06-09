#include "../headers/channel.hpp"
#include <climits>
chanel::chanel(){
    this->name_chanel = "" ;
    this->pass_chanel = "" ;
    // Mood : => :
    this->invitation = 0;
    this->t = 0;
    this->o = 0;
    this->limits = INT_MAX;
    this->password = 0; // pass 
    
}

void chanel::set_pass(std::string & str ){
    if(str.empty()){
        return ;
    }
    this->pass_chanel = str ;
    this->password = 1 ;
}


void chanel::set_name(std::string & str ){
    this->name_chanel = str ;
}
void chanel::add_CTOchanel(Client & client){
    this->all_Cchanel[client.get_Id()] = &client ;
}

std::string chanel::get_namechanel() const {
    return this->name_chanel ;
}

std::map <int , Client *>& chanel::get_opChanel(){
    return this->op_chanel ;
}

std::map <int , Client *>& chanel::get_All_Cchanel(){
    return this->all_Cchanel ;
}

std::string chanel::get_passwordChanel() const{
    return this->pass_chanel ;
}
bool chanel::get_password() const{
    return this->password;
}
int  chanel::get_limitchanell() const {
    return this->limits ;
}  
bool chanel::get_invetation() const{
    return this->invitation;
}
bool chanel::is_member(int fd )  {
    std::map <int , Client *> :: iterator it ;
    it = this->all_Cchanel.find(fd);  
    if(it != this->all_Cchanel.end()){
        return true;
    }
    return false;
};

std::set <std::string> &chanel::get_list() {
    return this->wait_list;
}


bool chanel::is_operator(int fd )  {
    std::map <int , Client *> :: iterator it ;
    it = this->op_chanel.find(fd);  
    if(it != this->op_chanel.end()){
        return true;
    }
    return false;
};
