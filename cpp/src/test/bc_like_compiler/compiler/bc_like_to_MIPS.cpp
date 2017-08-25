#include <iostream>
#include <cassert>
#include <string>
#include <memory>
#include <stack>
#include <tuple>
#include "../../../timeit.h"
#include "../../../treeregex_impl.h"
#include "../../../tree.h"
#include "../../../transformer.h"

using namespace std;

#define mt make_tuple

Transformer get_compiler();

void print_just_cgen(Tree* t, std::ostream& out){
	ListOfTrees* lt = dynamic_cast<ListOfTrees*>(t);
	if(!lt) return;

}

int main(int argc, char** argv){
	std::ios_base::sync_with_stdio(false);
	assert(argc==1);
	timeit _;
	std::istreambuf_iterator<char> begin(std::cin), end;
	std::string s(begin, end); s.pop_back(); // remove newline
	Tree* t = tree::parse(s);
	_.end("Reading in sexp");
	auto compiler = get_compiler();
	_.end("Building compiler");
	t = compiler(t);
	_.end("Compiling");

	t->print(std::cout); std::cout << '\n';
	return 0;

	/*string cgen = "cgen:";
	ostringstream oss;
	oss << ".text\n.globl __start\nj __start\n";
	print_just_cgen(t, oss);
	for_each(pre_begin(t), pre_end(t), [=,&oss,&cgen](const Tree&t){
			if(!t.is_type(Tree::STRING)){
				return;
			}
			const auto& str = t.get_str();
			if(!std::equal(str.begin(), str.begin()+std::min(cgen.length(), str.length()), cgen.begin())){
						//str.substr(0, cgen.size()) != cgen){
				//cout << str<< '\n';
				return;
			}
			for(int i = cgen.size(); i < str.length(); ++i){
				oss << str[i];
			} oss << '\n';
			//cout << str.substr(cgen.size()) << '\n';
		});
	oss << "outint:\nli $v0 1\nsyscall\naddiu $sp $sp 8\njr $ra\n";
	oss << "inint:\nli $v0 5\nsyscall\nmove $a0 $v0\naddiu $sp $sp 4\njr $ra\n";
	oss << "__start:\naddiu $sp $sp -4\njal main\nli $v0 10\nsyscall\n.data\n";
	std::cout << oss.str();
	_.end("Printing");*/
}

Transformer get_compiler(){
	Transformer t;
	shared_ptr<int> label_index = make_shared<int>(0);
	shared_ptr<stack<int>> while_labels = make_shared<stack<int>>();
	shared_ptr<vector<string>> offsets = make_shared<vector<string>>();
	auto get_offset = [=](const string& s){
		auto it = find(offsets->begin(), offsets->end(), s);
		int offset = distance(it, offsets->end());
		if(offset == 0){
			std::cerr << s << "could not be found!\n";
		}
		assert(offset != 0);
		return offset*4;
	};
	//t.add(

// Functions
		t.add("(%BC--:define(%BC--:([a-z]*)%)\\(@\\).*@%)", "(%end_function:(%cgen:$1:(%cgen:move \\$fp \\$sp%)(%push:\\$ra%)(%params:$2%)$3%)%)",true);
		t.add("(%BC--:define(%BC--:([a-z]*)%)\\(\\).*@%)", "(%end_function:(%cgen:$1:(%cgen:move \\$fp \\$sp%)(%push:\\$ra%)$2%)%)",true);
// macro for end of function
		t.add("(%end_function:@%)", [=](map<int,TreeSequence>&v, bool){
				offsets->clear();
			}, false);

// Parameters
		t.add("(%opt_argument_list:%)", "(%cgen:%)",true);
		t.add("(%params:(%BC--:([a-z]*)%)%)", "(%cgen:(%params:$1%)%)",true);
		t.add("(%params:(%BC--:@%)%)", "(%cgen:(%params:$1%)%)",true);
		t.add("(%params:(%BC--:@,@%)%)", "(%cgen:(%params:$1%)(%params:$2%)%)",true);
		t.add("(%params:([a-z]*)%)", [=](map<int, TreeSequence>& v){
				offsets->push_back(v[1].str());
				return tree::parse("(%cgen:%)");
			},true);

	// base case
		t.add("(%BC--:([0123456789][0123456789]*)%)","(%cgen:li \\$a0 $1%)",true);
// Syntactic Sugar
		t.add("(%BC--:@([^=!])=@%)","(%BC--:$1=(%BC--:(%BC--:$1%)$2$3%)%)",true);
		t.add("(%BC--:~@%)","(%BC--:(%BC--:-(%BC--:(%BC--:1%)%)%)^@%)",true);
		t.add("(%BC--:-@%)","(%BC--:(%BC--:(%BC--:0%)%)-@%)",true);
		t.add("(%BC--:@!=@%)","(%BC--:$1^$2%)", true);
		t.add("(%BC--:@==@%)","(%BC--:$1-$2%)", true);

// Expression ops
		t.add("(%BC--:@+@%)","(%cgen:$1(%push:\\$a0%)$2(%top:\\$t1%)(%cgen:add \\$a0 \\$t1 \\$a0%)(%pop%)%)",true);
		t.add("(%BC--:@-@%)","(%cgen:$1(%push:\\$a0%)$2(%top:\\$t1%)(%cgen:sub \\$a0 \\$t1 \\$a0%)(%pop%)%)",true);
		t.add("(%BC--:@\\*@%)","(%cgen:$1(%push:\\$a0%)$2(%top:\\$t1%)(%cgen:mul \\$a0 \\$t1 \\$a0%)(%pop%)%)",true);
		t.add("(%BC--:@//@%)","(%cgen:$1(%push:\\$a0%)$2(%top:\\$t1%)(%cgen:div \\$a0 \\$t1 \\$a0%)(%pop%)%)",true);
		t.add("(%BC--:@^@%)","(%cgen:$1(%push:\\$a0%)$2(%top:\\$t1%)(%cgen:xor \\$a0 \\$t1 \\$a0%)(%pop%)%)",true);
		t.add("(%BC--:(%BC--:([a-z]*)%)\\(\\)%)", "(%cgen:(%push:\\$fp%)(%call:$1%)%)",true);
		t.add("(%BC--:(%BC--:([a-z]*)%)\\((%BC--:@%)\\)%)", "(%cgen:(%push:\\$fp%)(%args:$2%)(%call:$1%)%)",true);
		t.add("(%BC--:(%BC--:([a-z]*)%)=@%)","(%cgen:$2(%save:$1%)%)",true);
		t.add("(%BC--:(%BC--:([a-z]*)%)%)", "(%load:$1%)",true);

// Arguments
		t.add("(%args:(%BC--:@,@%)%)", "(%cgen:$1(%push:\\$a0%)(%args:$2%)%)",true);
		t.add("(%args:(%BC--:@%)%)", "(%cgen:$1(%push:\\$a0%)%)",true);
// Returns
		t.add("(%BC--:return\\(@\\)%)", "(%cgen:$1(%return%)%)",true);
		t.add("(%BC--:if\\(@\\)@%)",[=](map<int, TreeSequence>& v){
				StringLeaf* s = new StringLeaf("if_label_"+to_string((*label_index)++));
				v[3] = {{s}};
			},"(%cgen:$1(%cgen:bnez \\$a0 $3%)$2(%cgen:$3:%)%)", true);
		t.add("(%BC--:while\\(@\\)@%)",[=](map<int, TreeSequence>&v){
				while_labels->push((*label_index)++); //
				v[3] = {{new StringLeaf("loop_label_"+to_string(while_labels->top()))}}; // 3
				v[4] = {{new StringLeaf("end_label_"+to_string(while_labels->top()))}}; // 4
			},"(%while_cgen:(%cgen:(%cgen:$3:%)$1(%cgen:beqz \\$a0 $4%)$2(%cgen:j $3%)(%cgen:$4:%)%)%)", true);
		t.add("(%BC--:break%)",[=](map<int, TreeSequence>& v){
				v[1] = {{new StringLeaf(
							"end_label_"+to_string(while_labels->top())
						)}}; // 1
				return v;
			}, "(%cgen:j $1%)", true);
		t.add("(%while_cgen:@%)", [=](map<int, TreeSequence>& v){
				while_labels->pop();}, "(%$1%)", false);

// things I inserted
		t.add("(%pop%)", "(%cgen:addiu \\$sp \\$sp 4%)",true);
		t.add("(%pop_args%)", [=](map<int, TreeSequence>&){
			int offset = (offsets->size()+2)*4;
			return tree::parse("(%cgen:addiu $sp $sp " + to_string(offset)+"%)");
		},true);
		t.add("(%load:(.*)%)", [=](map<int, TreeSequence>& c){
			auto off = get_offset(c[1].str());
			return tree::parse("(%cgen:lw $a0 " + to_string(off)+"($fp)%)");
		},true);
		t.add("(%save:(.*)%)", [=](map<int, TreeSequence>& c){
			auto off = get_offset(c[1].str());
			return tree::parse("cgen:sw $a0 " + to_string(off)+"($fp)%)");
		},true);
		t.add("(%top:(.*)%)", "(%cgen:lw $1 4(\\$sp)%)",true);
		//t.add("(%push:(.*)%)", "(%cgen:(%cgen:sw $1 0(\\$sp)%)(%cgen:addiu \\$sp \\$sp -4%)%)",true);
		t.add("(%call:(.*)%)", "(%cgen:jal $1%)", true);  // At this point all args and fp are on stack
		t.add("(%return%)", "(%cgen:(%top:\\$ra%)(%pop_args%)(%cgen:lw \\$fp 0(\\$sp)%)(%cgen:jr \\$ra%)%)", true);
		//);
/*
		*/
	return t;
}
