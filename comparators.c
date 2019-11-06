#include "comparators.h"
#include "Record.h"

int (*comparator[8])(Record*, Record*) = {id_cmp, name_cmp, surname_cmp, home_addr_cmp, home_num_cmp, city_cmp, mail_cmp, salary_cmp};


int id_cmp(Record* r1, Record* r2){
    return ( r1->id - r2->id );
}

int name_cmp(Record* r1, Record* r2){
    return strcmp(r1->name, r2->name);
}

int surname_cmp(Record* r1, Record* r2){
    return strcmp(r1->surname, r2->surname);
}

int home_addr_cmp(Record* r1, Record* r2){
    return strcmp(r1->home_address, r2->home_address);
}

int home_num_cmp(Record* r1, Record* r2){
    return ( r1->home_number - r2->home_number );
}

int city_cmp(Record* r1, Record* r2){
    return strcmp(r1->city, r2->city);
}

int mail_cmp(Record* r1, Record* r2){
    return strcmp(r1->mail_sector, r2->mail_sector);
}


int salary_cmp(Record* r1, Record* r2){
    return ( r1->salary - r2->salary );
}

void swap(Record** r1, Record** r2){
    Record* temp = *r1;
    *r1 = *r2;
    *r2 = temp;
}