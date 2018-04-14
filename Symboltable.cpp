#include "Gamma.h"

// table
void table::insert_subtable(std::string str, void * ptr) {
	sub_table.insert(std::make_pair(str, ptr));
}
void table::insert_variable(std::string str, llvm::Value *var) {
	var_table.insert(std::make_pair(str, var));
}
std::string table::lookup(std::string str) {
	if (var_table.find(str) == var_table.end()) {
		return "";
	}
	return str;
}

// class_table
class_table::class_table(std::string id) {
	this->class_id=id;
}
std::string class_table::get_id() {
	return this->class_id;
}

// function_table ==========================
fuc_table::fuc_table(std::string id){
	this->fuc_id = id;
}

std::string fuc_table::get_id() {
	return this->fuc_id;
}

void fuc_table::insert_variable(std::string str, llvm::Value *var){
	var_table.insert(std::make_pair(str, var));
}
