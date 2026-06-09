#include "../headers/execute.hpp"


void executePass::executeCmd(Server& server, Client& client, const std::vector<std::string>& args) {
    if (client.get_Isregister()) {
        client.getSendBuffer() += "462 " + client.get_nick() + " :Unauthorized command (already registered)\r\n";
        return; 
    }
    if (client.get_fPass()) {
        client.getSendBuffer() += "462 * :Unauthorized command (already registered)\r\n"; 
        return;
    }
    if (args.empty()) {
        client.getSendBuffer() += "461 * PASS :Not enough parameters\r\n"; 
        return;
    }
    if (server.get_pass() != args[0]) {
        client.getSendBuffer() += "464 * :Password incorrect\r\n";
        client.set_Close(1);
        return ;
    }
    else {
        client.set_fPaa(true);
    }
}

void executeNick::executeCmd(Server& server, Client& client, const std::vector<std::string>& args)
{
    if (client.get_fPass() == 0)
    {
        client.getSendBuffer() += "451 * :You have not registered\r\n";
        return ;
    }
    if (args.empty())
    {
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += "431 " + str +" Not enough parameters\r\n"; 
        return ;
    }
    if (args[0][0] == '#' || args[0][0] == '&' || args[0][0] == ':')
    {
        std::string str = client.get_nick().empty() ? "*" : client.get_nick();
        client.getSendBuffer() += "432 " + str + " " + args[0] + " :Erroneous nickname\r\n";
        return ;
    }
    if (client.get_fNake() && client.get_nick() == args[0])
        return ;
    else
    {
        std::map < int , Client>::iterator temp ;
        for (temp =  server.get_mapClient().begin() ; temp!=server.get_mapClient().end() ; ++temp ){
            if(temp->second.get_nick() == args[0]){
                client.getSendBuffer() += "433 * " + args[0] + " :Nickname is already in use\r\n";
                return ;
            }
        }
        std::string old_nick = client.get_nick() ; 
        client.set_nake(args[0]);
        client.set_fNake(true);
        if(client.get_Isregister() == true){
            client.getSendBuffer() += ":" + old_nick + " NICK :" + args[0] + "\r\n";
            return ;
        }
        if(client.get_fUser() && client.get_fNake() && client.get_fPass()){
            client.set_Isregister(true);
            client.getSendBuffer() += "001 " + args[0] + " :Welcome to the Internet Relay Network " + args[0] + "\r\n";
            return ;
        }
    }
}

void executeUser::executeCmd(Server& server, Client& client, const std::vector<std::string>& args) {
    if(client.get_fPass() == 0){
        client.getSendBuffer() += "451 * :You have not registered\r\n";
        return ;
    }
    if (args.empty()) {
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += "431 " + str +" Not enough parameters\r\n"; 
        return ;
    }
    if(args.size() < 4){
          std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += "461 " + str + " USER :Not enough parameters\r\n";
        return ;
    }
    if(client.get_fUser() || client.get_Isregister()){
        std::string prefix = client.get_nick().empty() ? "*" : client.get_nick();
        client.getSendBuffer() += "462 " + prefix + " :Unauthorized command (already registered)\r\n";
        return ;
    }else {
        client.set_user(args[0]);
        client.set_Realname(args[3]);
        client.set_fUser(true);
        if(client.get_fUser() && client.get_fNake() && client.get_fPass()){
            client.set_Isregister(true);
            client.getSendBuffer() += "001 " + args[0] + " :Welcome to the Internet Relay Network " + args[0] + "\r\n";
            return ;
        }
    }
}

void executeQuit::executeCmd(Server& server, Client& client, const std::vector<std::string>& args) {
    client.set_Close(1);
    client.getRecvBuffer().clear();
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss;
    ss << str.c_str() ;
    std::string token;
    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return (tokens);
}

void executePrivmsg::executeCmd(Server& server, Client& client, const std::vector<std::string>& args) {
    if(client.get_Isregister() == false){
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += ":localhost 451 " + str + " :You have not registered\r\n";
        return ;
    }
    if(args.empty()){
        client.getSendBuffer() += ":localhost 411 " + client.get_nick() + " :No recipient given (PRIVMSG)\r\n";
        return ;
    }
    if(args.size() < 2 || args[1].empty()){
        client.getSendBuffer() += ":localhost 412 " + client.get_nick() + " :No text to send\r\n";
        return;
    }
    std::vector <std::string > newarg  = split(args[0] , ',');
    if(newarg.size() > 5){
        client.getSendBuffer() += ":localhost 407  :Too many recipients. Message not delivered.\r\n";
        return ;
    }
    int i = 0  , p;
    while (i < newarg.size())
    {
        p = 0;
        if(newarg[i][0] == '#' || newarg[i][0] == '&'){
            std::string packet = ":" + client.get_nick() + "!" + client.get_user() + "@"+ client.get_host()+" PRIVMSG " + newarg[i] + " :" + args[1] + "\r\n";
            std::map <std::string, chanel > :: iterator it  = server.get_Chanel().find(newarg[i]);
            if(it != server.get_Chanel().end()){
                if(it->second.is_member(client.get_Id()) == 0){
                    client.getSendBuffer() += ":localhost 404 " + client.get_nick() + " " + args[0] + " :Cannot send to channel\r\n";
                    return ;
                }
                std::map < int, Client * > :: iterator itt ;
                for(itt = it->second.get_All_Cchanel().begin() ; itt != it->second.get_All_Cchanel().end() ;++itt){
                    if (itt->first == client.get_Id()) {
                        continue; 
                    }
                    p = 1 ;
                    itt->second->getSendBuffer() += packet;
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = itt->first;
                    epoll_ctl(server.get_fdeppol(), EPOLL_CTL_MOD, itt->first, &ev);
                }
            }  
        }
        else
        {
            std::map<int , Client> :: iterator it ;
            for(it = server.get_mapClient().begin() ; it != server.get_mapClient().end()  ;++it){
                if(it->second.get_nick() == newarg[i]){
                    p = 1 ;
                    std::string packet = ":" + client.get_nick() + "!" + client.get_user() + "@"+ client.get_host()+" PRIVMSG " + newarg[i] + " :" + args[1] + "\r\n";
                    it->second.getSendBuffer() += packet;
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = it->first;
                    epoll_ctl(server.get_fdeppol(), EPOLL_CTL_MOD, it->first, &ev);
                    break;        
                }
            }
        }
        if(p == 0) {
            client.getSendBuffer() += ":localhost 401 " + client.get_nick() + " " + newarg[i] + " :No such nick/channel\r\n";
        }
        i++ ;
    }
    return;
}

void executeJoin::executeCmd(Server& server, Client& client, const std::vector<std::string>& args) {    
    if(client.get_Isregister() == false){
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += ":localhost 451 " + str + " :You have not registered\r\n";
        return ;
    }
    if(args.empty() || args[0].empty()){
        client.getSendBuffer() += ":localhost 461 " + client.get_nick() + " JOIN :Not enough parameters\r\n";
        return ;
    }
    std::vector <std::string > newarg  = split(args[0] , ',');
    std::vector <std::string > newPas ;
    if(args.size() == 1){
        newPas.clear() ;
    } else {
        newPas = split(args[1] , ','); 
    }
    int  i = 0 ;
    while (i < newarg.size())
    {
        if(newarg[i].size() < 2 || (newarg[i][0] != '#' && newarg[i][0] != '&')){
            client.getSendBuffer() += ":localhost 403 " + client.get_nick() + " " + newarg[i] + " :No such channel\r\n";
            i++;
            continue;
        }
        std::map<std::string, chanel> ::iterator it = server.get_Chanel().find(newarg[i]);
        if(it == server.get_Chanel().end()){
            if(i < newPas.size()){
                server.cretionChanel(newarg[i],newPas[i],client);
            }else{
                server.cretionChanel(newarg[i],"",client);
            }
            server.get_Chanel()[newarg[i]].get_opChanel()[client.get_Id()] = &client ;
            std::string packet = ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " JOIN :" + newarg[i] + "\r\n";
            std::string msg2 = ":ft_irc_default 353 " + client.get_nick() + " = " + newarg[i] + " :@" + client.get_nick() + "\r\n";
            std::string msg3 = ":ft_irc_default 366 " + client.get_nick() + " " + newarg[i] + " :End of /NAMES list.\r\n";
            client.getSendBuffer() = packet + msg2 + msg3;
            i++;
        }   
        else {
            std::stringstream buff ;
            std::map<std::string, chanel> ::iterator iter = server.get_Chanel().find(newarg[i]);
            if(iter->second.is_member(client.get_Id()) == 0){
                iter->second.get_All_Cchanel()[client.get_Id()] = &client ;
                std::map<int, Client *> ::iterator index ;
                std::string join_msg = ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " JOIN :" + server.get_Chanel()[newarg[i]].get_namechanel() + "\r\n";
                for(index = iter->second.get_All_Cchanel().begin() ; index != iter->second.get_All_Cchanel().end() ; ++index){
                    index->second->getSendBuffer() += join_msg;
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLOUT;
                    ev.data.fd = index->first;
                    epoll_ctl(server.get_fdeppol(), EPOLL_CTL_MOD, index->first, &ev);
                }
                buff << ":ft_irc_default 353 " << client.get_nick() << " = " << iter->second.get_namechanel() << " :";
                for(index = iter->second.get_All_Cchanel().begin() ; index != iter->second.get_All_Cchanel().end() ; ++index){
                    if(server.get_Chanel()[newarg[i]].get_opChanel().find(index->second->get_Id()) != server.get_Chanel()[newarg[i]].get_opChanel().end() ){
                        buff << "@" ;
                    }
                    buff << index->second->get_nick() << " "; 
                }
                buff << "\r\n";
                std::string msg366 = ":ft_irc_default 366 " + client.get_nick() + " " + server.get_Chanel()[newarg[i]].get_namechanel() + " :End of /NAMES list.\r\n";
                client.getSendBuffer() += buff.str() ;
                client.getSendBuffer() +=  msg366 ;
            }
            i++;
        }
    }
}

void executeKick::executeCmd(Server& server, Client& client, const std::vector<std::string>& args){
    if(client.get_Isregister() == false){
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += ":localhost 451 " + str + " :You have not registered\r\n";
        return ;
    }
    if(args.empty()){
        client.getSendBuffer() += ":localhost 411 " + client.get_nick() + " :No recipient given (KICK)\r\n";
        return ;
    }
    if(args.size() < 2 ){
        client.getSendBuffer() += ":localhost 412 " + client.get_nick() + " :No text to send\r\n";
        return;
    }
    std::vector <std::string> newchanel = split(args[0],',');
    std::vector <std::string> newUser  = split(args[1],',');
    if(newchanel.size() == 1 && newUser.size() >= 1) {
        if(newchanel[0][0] != '#' && newchanel[0][0] != '&'){
            client.getSendBuffer() += ":localhost 403 " + client.get_nick() + " " + newchanel[0] + " :No such channel\r\n";
            return ;
        }
        std::map <std::string, chanel> :: iterator it = server.get_Chanel().find(newchanel[0]);
        if(it != server.get_Chanel().end()){
            if(it->second.is_operator(client.get_Id()) == 0){
                client.getSendBuffer() += ":localhost 482 " + client.get_nick() + " " + newchanel[0] + " :You're not channel operator\r\n";
                return ;
            }
            std::map <int, Client *> ::iterator itt ;
            int i = 0 , p ;
            while (i < newUser.size())
            {
                p = 0;
                for(itt = it->second.get_All_Cchanel().begin(); itt != it->second.get_All_Cchanel().end(); ++itt){
                    if(itt->second->get_nick() == newUser[i]){
                        p = 1;
                        std::string str = (args.size() > 2) ? args[2] : client.get_nick() ;
                        std::string packet = ":" + client.get_nick() + "!" +client.get_user()+ "@" +client.get_host()+ " KICK " + newchanel[0]+ " " +newUser[i]+" :"+str+"\r\n";
                        std::map <int, Client *>::iterator broad_it;
                        for (broad_it = it->second.get_All_Cchanel().begin(); broad_it != it->second.get_All_Cchanel().end(); ++broad_it) {
                            broad_it->second->getSendBuffer() += packet;
                            struct epoll_event ev;
                            ev.events = EPOLLIN | EPOLLOUT;
                            ev.data.fd = broad_it->first;
                            epoll_ctl(server.get_fdeppol(), EPOLL_CTL_MOD, broad_it->first, &ev);
                        }
                        if(it->second.is_operator(itt->second->get_Id()) == 1){
                            it->second.get_opChanel().erase(itt->first);
                        }
                        it->second.get_All_Cchanel().erase(itt);
                        break;
                    }
                }
                if(p == 0){
                    client.getSendBuffer() += ":localhost 441 " + client.get_nick() + " " + newUser[i] + " " + args[0] + " :They aren't on that channel\r\n" ;
                }
                i++ ;
            }
            if (it->second.get_All_Cchanel().empty()){
                server.get_Chanel().erase(it);
            }
        }else {
             client.getSendBuffer() += ":localhost 403 " + client.get_nick() + " " + newchanel[0] + " :No such channel\r\n";
        }
    }
}

void executeInvite::executeCmd(Server& server, Client& client, const std::vector<std::string>& args){
    if(client.get_Isregister() == false){
        std::string str = client.get_nick().empty() ? "*" : client.get_nick() ;
        client.getSendBuffer() += ":localhost 451 " + str + " :You have not registered\r\n";
        return ;
    }
    if(args.empty()){
        client.getSendBuffer() += ":localhost 411 " + client.get_nick() + " :No recipient given (KICK)\r\n";
        return ;
    }
    if (args.size() < 2) {
        client.getSendBuffer() += ":localhost 461 " + client.get_nick() + " INVITE :Not enough parameters\r\n";
        return;
    }
    std::map <std::string, chanel> :: iterator it = server.get_Chanel().find(args[1]);
    if(it != server.get_Chanel().end()){
        if(it->second.is_member(client.get_Id()) == 0){
            client.getSendBuffer() += ":localhost 442 " + client.get_nick() + " " + args[1] + " :You're not on that channel\r\n";
            return;
        }
        if(it->second.is_operator(client.get_Id()) == 0){
            client.getSendBuffer() += ":localhost 482 " + client.get_nick() + " " + args[1] + " :You're not channel operator\r\n";
            return;
        }
        std::map <int , Client> :: iterator All_client ; int p = 0 ;
        for(All_client = server.get_mapClient().begin() ; All_client != server.get_mapClient().end() ; ++All_client){
            if(All_client->second.get_nick() == args[0]){
                p = 1;
                break;
            }
        }
        if(p == 0){
            client.getSendBuffer() += ":localhost 401 " + client.get_nick() + " " + args[0] + " :No such nick/channel\r\n";
            return;
        }
        std::map <int , Client*> :: iterator it_client ;
        for(it_client = it->second.get_All_Cchanel().begin() ;it_client !=it->second.get_All_Cchanel().end() ; ++it_client){
            if(it_client->second->get_nick() == args[0]){
                client.getSendBuffer() += ":localhost 443 " + client.get_nick() + " " + args[0] + " " + args[1] + " :is already on channel\r\n";
                return;
            }
        }
        it->second.get_list().insert(args[0]);
        std::string invite_packet = ":" + client.get_nick() + "!" + client.get_user() + "@" + client.get_host() + " INVITE " + All_client->second.get_nick() + " :" + args[1] + "\r\n";
        All_client->second.getSendBuffer() += invite_packet;
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLOUT;
        ev.data.fd = All_client->second.get_Id();
        epoll_ctl(server.get_fdeppol(), EPOLL_CTL_MOD, All_client->second.get_Id(), &ev);
        client.getSendBuffer() += ":localhost 341 " + client.get_nick() + " " + All_client->second.get_nick() + " " + args[1] + "\r\n";
    }
    else{
        client.getSendBuffer() += ":localhost 403 " + client.get_nick() + " " + args[1] + " :No such channel\r\n";
        return;
    }
}
