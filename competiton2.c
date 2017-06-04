#include <stdio.h>
#include <stdlib.h>
#include "trader_bot.h"

//calculate the current total weight of the cargo the robot has
int get_current_cargo_weight(struct bot *b);
//calculate the current total volume of the cargo the robot has
int get_current_cargo_volume(struct bot *b);
//find the number of locations of the world
int get_world_length(struct bot *b);
//find the nearst shop and return the value
int find_nearst_buy_shop(struct bot *b);
//find the nearset fule station to fuel up
int find_the_nearst_fuel_station(struct bot *b);
//find the nearast shop which could sell the same type of cargo
int find_the_nearst_sell_shop(struct bot *b);
//judge dump or not
int do_dump(struct bot *b);

char *get_bot_name(void) {
    return "CcLl Bot";
}

void get_action(struct bot *b, int *action, int *n) {
    int a=0;
    //costmoneyNeeded is the money would be cost if the robot wants to buy something
    int costmoneyNeeded=0;
    //to indicate in this position to buy fuel or not
    int buy_fuel=0;
    struct cargo* buyercargo=b->cargo;
   // struct cargo* newcargo;
    //sellcargo is used to simplify the code
    struct cargo* sellcargo=b->cargo;
    //the item want to buy is already in the cargo list or not
    int alreadybuyitemType=0;
    //based on the location find the action we need
    //this cargo size is used to generate the new cargo in the robot when the robot buy a new type of cargo
    int cargo_size=sizeof(struct cargo);

    switch(b->location->type){
        case LOCATION_START :
            *action=ACTION_MOVE;
            //at the start point, find the nearest shop to buy staff
            *n=find_nearst_buy_shop(b);
            //if the fuel is not enough, go and get fuel added
            if(abs(*n)>(b->fuel)){
                *n=find_the_nearst_fuel_station(b);
            }
            return;
        case LOCATION_BUYER :
            *action=ACTION_BUY;
            break;
        case LOCATION_PETROL_STATION:
            *action=ACTION_BUY;
            buy_fuel=1;
            break;
        case LOCATION_SELLER:
            *action = ACTION_SELL;
            break;
        case LOCATION_DUMP:
            *action = ACTION_DUMP;
            break;
        case LOCATION_OTHER:
            *action = ACTION_MOVE;
            *n =1;
            return;
    }

    //based on the action to update something
    switch(*action){
        case ACTION_MOVE:

            break;
        case ACTION_BUY:

            //actually do the buy action
            if(buy_fuel != 1){
                //buy other cargo
                //by default buy all the cargo in this staff
                *n=b->location->quantity;
                //in this action, n would be number of items, the robot wants to buy
                costmoneyNeeded=(b->location->price)*(*n);
                //if costmoneyNeeded greater than the cash the robot has, it couldn't buy so many items
                if(costmoneyNeeded>(b->cash)){
                    //calculate the number of items the robot could buy
                    *n=(b->cash)/(b->location->price);
                }
                //the items would exceed the bot's weight
                if(((b->location->commodity->weight)*(*n)+get_current_cargo_weight(b))>b->maximum_cargo_weight){
                    //based on the weight limitation to calculate the number of items the robot could buy
                    *n=(b->maximum_cargo_weight-get_current_cargo_weight(b))/(b->location->commodity->weight);
                }
                //the items would exceed the bot's volume
                if(((b->location->commodity->volume)*(*n)+get_current_cargo_volume(b))>b->maximum_cargo_volume){
                    //based on the weight limitation to calculate the number of items the robot could buy
                    *n=(b->maximum_cargo_volume-get_current_cargo_weight(b))/(b->location->commodity->volume);
                }
                if((*n) == 0){
                    //couldn't buy anything
                    *action=ACTION_MOVE;
                    //find the neareat shop to sell cargo
                    *n=find_the_nearst_sell_shop(b);
                    //if the fuel is not enough, go and get fuel added
                    if(abs(*n)>(b->fuel)){
                        *n=find_the_nearst_fuel_station(b);
                    }
                    return;
                }
                //update the cash after buy the items
                b->cash -=(b->location->price)*(*n);
                //add the item the user buy into the cargo
                //if the buyer item is already in the cargo list
                while(buyercargo != NULL){
                    if((*b->location->commodity->name) == (*buyercargo->commodity->name) ){
                        //the buyer item is already in the cargo list
                        //only need to increase the number
                        alreadybuyitemType=1;
                        //update the number of this type of cargo
                        buyercargo->quantity +=(*n);
                        break;
                    }
                    buyercargo=buyercargo->next;
                }
                if(alreadybuyitemType != 1){
                    //the buyer item is not in the cargo list
                    //create new one and add into the chain
                    a=(*n);
                    struct cargo* newcargo=(struct cargo*)malloc(sizeof(cargo_size));
                    newcargo->next=NULL;
                    newcargo->commodity=(b->location->commodity);
                    newcargo->quantity=a;
                    //add this new cargo into the end of the cargo list
                    buyercargo=b->cargo;
                    while(buyercargo->next != NULL){
                        buyercargo=buyercargo->next;
                    }
                    buyercargo->next=newcargo;
                }
            }else{
                //buy fuel
                //first try to add to the fuel_tank_size
                *n=b->fuel_tank_capacity-b->fuel;
                if(*n>b->location->quantity){
                    //should be the number of items left in the location
                    *n=b->location->quantity;
                }
                if(b->cash < (b->location->price)){
                    *n =0;
                }
                if((*n) == 0){
                    //couldn't buy anything
                    *action=ACTION_MOVE;
                    //find the neareat shop to sell cargo
                    *n=find_the_nearst_sell_shop(b);
                    //if the fuel is not enough, go and get fuel added
                    if(abs(*n)>(b->fuel)){
                        *n=find_the_nearst_fuel_station(b);
                    }
                    return;
                }
                if((*n)*(b->location->price)>b->cash){
                    //if not enough money
                    //update the fuel
                    b->fuel +=(b->cash)/(b->location->price);
                    //update the cash
                    b->cash -= ((b->cash)/(b->location->price))*((b->location->price));
                }else{
                    //if enough money
                    //update the fuel
                    b->fuel +=*n;
                    //update the cash
                    b->cash -= (*n)*(b->location->price);
                }
            }
            break;
        case ACTION_SELL:
            //sellcargo
            while(sellcargo != NULL){
                //the robot has something to sell
                //check if the cargo is the same with the seller in this location
                if((*sellcargo->commodity->name) == (*b->location->commodity->name)){
                    //find the item to sell
                    *n = sellcargo->quantity;
                    //update the cargo and cash
                    if((*n)>b->location->quantity){
                        //if the cargo the robot want to sell greater than the cargo the localtion want to buy
                        (*n)=b->location->quantity;
                    }
                    //update the cash
                    b->cash += (*n)*b->location->price;
                    //update the cargo
                    sellcargo->quantity -= (*n);
                    //break the while loop
                    return;
                }
                sellcargo=sellcargo->next;
            }
            //nothing to sell
            *action=ACTION_MOVE;
            //find the neareat shop to sell cargo
            *n=find_the_nearst_sell_shop(b);
            //if the fuel is not enough, go and get fuel added
            if(abs(*n)>(b->fuel)){
                *n=find_the_nearst_fuel_station(b);
            }
            return;
        case ACTION_DUMP:
            //if in the cargo nothing could sell
            //dump everything
            if(do_dump(b)){
                b->cargo = NULL;
                return ;
            }
            //if bag is empty go and buy something
            if(b->cargo == NULL){

            }
            //if have something to sell
            *action = ACTION_MOVE;
            //find the neareat shop to sell cargo
            *n=find_the_nearst_sell_shop(b);
            //if the fuel is not enough, go and get fuel added
            if(abs(*n)>(b->fuel)){
                *n=find_the_nearst_fuel_station(b);
            }
            break;
    }

}

//find the nearst shop and return the value
int find_nearst_buy_shop(struct bot *b){
    int a=0;
    int *n=&a;
    struct location *clocation=b->location;
    int world_length=0;

    while(clocation->type != LOCATION_BUYER){
        (*n) = (*n)+1;
        clocation=clocation->next;
    }
    world_length=get_world_length(b);
    //if n > world_length/2, move to the opposite direction
    if(world_length < 2*(*n)){
       *n=(*n)- world_length;
    }
    return *n;

}

//find the length of the world
int get_world_length(struct bot *b){
    struct location *bot_location = b->location;
    int n_printed = 0; // sentinal variable for loop

    // locations are linked in a circular list
    struct location *l =  bot_location;
    while (n_printed == 0 || l != bot_location) {
        l = l->next;
        n_printed++;
    }
    return n_printed;
}


//find the nearast shop which could sell the same type of cargo
int find_the_nearst_sell_shop(struct bot *b){
    int a=0;
    int* n=&a;
    struct cargo * ccargo=b->cargo;
    struct location * clocation=b->location;
    int start=0;
    //when
    while(ccargo != NULL){
        //find the position
        start=0;
        clocation=b->location;
        while(start == 0 || clocation !=b->location ){
            if(clocation->type == LOCATION_SELLER){
                if((*clocation->commodity->name) == (*ccargo->commodity->name)){
                    //do have the thing to sell
                    if(clocation->quantity > 0){
                        *n = start;
                        if(get_world_length(b) < 2*(*n)){
                            *n -= get_world_length(b);
                        }
                        return *n;
                    }
                }
            }
            clocation=clocation->next;
            start+=1;
        }
        ccargo=ccargo->next;
    }
    // if not find the shop could sell cargo
    *n =1;
    return *n;
}


//find the nearset fule station to fuel up
int find_the_nearst_fuel_station(struct bot *b){
    int a=0;
    int * n=&a;
    struct location *clocation=b->location;
    int world_length=0;

    while(clocation->type != LOCATION_PETROL_STATION){
        *n = *n+1;
        clocation=clocation->next;
    }
    world_length=get_world_length(b);
    //if n > world_length/2, move to the opposite direction
    if(world_length < 2*(*n)){
        *n=(*n)- world_length;
    }
    return *n;

}

int get_current_cargo_weight(struct bot *b){
    int weight=0;
    //ccargo is the current cargo in the computation
    struct cargo *ccargo=b->cargo;
    //if the robot has no cargo,return weight 0
    if(ccargo == NULL){
        return 0;
    }
    //add all the weight of the cargo the robot has
    while(ccargo->next != NULL){
        weight +=(ccargo->quantity)*(ccargo->commodity->weight);
        ccargo=ccargo->next;
    }
    //add the weight of last cargo
    weight +=(ccargo->quantity)*(ccargo->commodity->weight);
    return weight;
}

int get_current_cargo_volume(struct bot *b){
    int volume=0;
    //ccargo is the current cargo in the computation
    struct cargo *ccargo=b->cargo;
    //if the robot has no cargo,return weight 0
    if(ccargo == NULL){
        return 0;
    }
    //add all the weight of the cargo the robot has
    while(ccargo->next != NULL){
        volume +=(ccargo->quantity)*(ccargo->commodity->volume);
        ccargo=ccargo->next;
    }
    //add the weight of last cargo
    volume +=(ccargo->quantity)*(ccargo->commodity->volume);
    return volume;
}

//judge dump or not
int do_dump(struct bot *b){
    struct cargo * ccargo = b->cargo;
    struct location * clocation=b->location;
    int start=0;
    while(ccargo != NULL ){
        //find the position
        start=0;
        clocation=b->location;
        while(start == 0 || clocation !=b->location){
            if(clocation->type == LOCATION_SELLER){
                if((*clocation->commodity->name) == (*ccargo->commodity->name)){
                    if(clocation->quantity > 0){
                        return 0;
                    }
                }
            }
            clocation=clocation->next;
            start +=1;
        }

        ccargo = ccargo->next;
    }
    return 1;

}
