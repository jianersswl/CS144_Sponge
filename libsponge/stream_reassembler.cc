#include "stream_reassembler.hh"
#include <fstream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ofstream fout;

StreamReassembler::StreamReassembler(const size_t capacity) :
_first_unread(0),
_unassembled_bytes_size(0),
_eof_index(-1),
_unassembled_buffer(),
_output(capacity), 
_capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    fout.open("test.log", ios::app);
    fout << "*********** receive stream: " << data << "**************" << endl;
    // 无视已经写进bytestream里的内容
    if(index + data.length() < _first_unread){
        return;
    }
    fout << "_eof_index: " << _eof_index << endl;
    if(eof == true){
        _eof_index = index + data.length() - 1;
        // stream_out().end_input();
    }
    fout << "_eof_index: " << _eof_index << endl;
    fout << "_unassembled_bytes_size: " << _unassembled_bytes_size << endl;
    fout << "*********************************************************" << endl;
    fout.close();
    // check overflow
    // if(_unassembled_bytes_size + _output.buffer_size() == _capacity){
        // return;
    // }

    Tuple bag = {data, index};

    if(_unassembled_bytes_size == 0){
        _unassembled_buffer.push_front(bag);
        _unassembled_bytes_size += data.length();
    }else{
        merge_overlap(bag);
    }
    check_overflow();
    write_bytes();

}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes_size; }

bool StreamReassembler::empty() const { return _unassembled_bytes_size==0; }

void StreamReassembler::check_overflow() {
    fout.open("test.log", ios::app);
    fout << "%%%%%%%%%%%%%%%%%%%%%% check overflow %%%%%%%%%%%%%%%%%%%%%%" << endl;
    string ss;
    // 消除已经write的字节
    auto head = _unassembled_buffer.begin();
    if(head->index < _first_unread){
        head->data = (head->data).substr(_first_unread-head->index, (head->data).length() - (_first_unread - head->index));
        _unassembled_bytes_size -= (_first_unread - head->index);
    }
    // 消除overflow的尾字节
    for(list<Tuple>::iterator it = _unassembled_buffer.begin(); it!=_unassembled_buffer.end(); it++){ss+=it->data;}
    fout << "overflow buffer: " << ss << endl;
    while(_unassembled_bytes_size + _output.buffer_size() > _capacity){
        fout << "cur _unassembled_bytes_size: " << _unassembled_bytes_size << endl;
        fout << "cur _output.buffer_size(): " << _output.buffer_size() << endl;
        auto tail = _unassembled_buffer.rbegin();
        fout << "last node: " << tail->data << endl;
        if(_unassembled_bytes_size + _output.buffer_size() - (tail->data).length() >= _capacity){
            fout << "pop last: " << tail->data << endl;
            _unassembled_bytes_size -= (tail->data).length();
            _unassembled_buffer.pop_back();
        }else{
            size_t drop = _unassembled_bytes_size + _output.buffer_size() -  _capacity;
            tail->data = (tail->data).substr(0, (tail->data).length()-drop);
            _unassembled_bytes_size -= drop;
            fout << "split last: " << _unassembled_buffer.back().data << endl;
        }
    }
    string ss1;
    for(list<Tuple>::iterator it = _unassembled_buffer.begin(); it!=_unassembled_buffer.end(); it++){ss1+=it->data;}
    fout << "checked buffer: " << ss1 << endl;
    fout.close();
}
void StreamReassembler::write_bytes(){
    // 考虑到只有列表头才有机会被写到bytestream里面
    list<Tuple>::iterator it = _unassembled_buffer.begin();
    if(_first_unread >= it->index){
        // loooooooooooooooooooooooooooog
        fout.open("test.log", ios::app);
        fout << "++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
        fout << "write bytes: " << (it->data).substr(_first_unread - it->index, it->index + (it->data).length()-_first_unread) << endl;
        fout << "_eof_index: " << _eof_index << endl;
        fout << "write end index: " << (it->index + (it->data).length() - 1) <<endl;
        
        // 不用考虑头已经写过？
        _output.write((it->data).substr(_first_unread - it->index, it->index + (it->data).length()-_first_unread));
        
        if((it->index + (it->data).length() - 1) == _eof_index){
            stream_out().end_input();
        }
        
        _unassembled_bytes_size -= it->index + (it->data).length() - _first_unread;
        _first_unread = it->index + (it->data).length();
        _unassembled_buffer.erase(it);
        fout << "_unassembled_bytes_size: " << _unassembled_bytes_size << endl;
        fout << "++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
        fout.close();
    }
}
void StreamReassembler::merge_overlap(Tuple newRecieve) {
    fout.open("test.log", ios::app);
    fout << "!!!!!!!!!!!! merge function !!!!!!!!!!!!!!!" << endl;
    list<Tuple>::iterator it = _unassembled_buffer.begin();
    

    // 找到第一个有可能产生merge的节点
    for (; it != _unassembled_buffer.end(); ++it) {
        if(it->index + (it->data).length() - 1 >= newRecieve.index) {
            break;
        }
    }
    fout << it->data << endl;
    fout << "@@@@@@@@@@@@@@@@@@@@@" << endl;
    // 没有merge，直接插到队尾
    if(it == _unassembled_buffer.end()){
        fout << "insert end" << endl;
        _unassembled_buffer.emplace_back(newRecieve);
        _unassembled_bytes_size += newRecieve.data.length();
    } else {
        // 插入后向后merge
        fout << "merge next" << endl;
        it = _unassembled_buffer.insert(it, newRecieve);
        list<Tuple>::iterator pre = it;
        it++;
        _unassembled_bytes_size = _unassembled_bytes_size + (pre->data).length();
        for(; it != _unassembled_buffer.end(); ++it){
            fout << "node " << it->data << endl;
            size_t pre_start = pre->index;
            size_t pre_end = pre_start + (pre->data).length() - 1;
            size_t it_start = it->index;
            size_t it_end = it_start + (it->data).length() - 1;
            fout << "pre_start: " << pre_start <<endl;
            fout << "pre_end: " << pre_end <<endl;
            fout << "it_start: " << it_start <<endl;
            fout << "it_end: " << it_end <<endl;
            fout << "_unassembled_bytes_size: " << _unassembled_bytes_size <<endl;
            // 已经不能再merge
            if(pre_end + 1 < it_start){
                break;
            }
            // pre 包含 it
            if(pre_start<=it_start && pre_end>=it_end){
                fout << "pre 包含 it" << endl;
                _unassembled_bytes_size = _unassembled_bytes_size - (it->data).length();
                it->data = pre->data;
                it->index = pre->index;
            // it 包含 pre
            }else if(pre_start>=it_start && pre_end<=it_end){
                fout << "it 包含 pre" << endl;
                _unassembled_bytes_size = _unassembled_bytes_size - (pre->data).length();
            // pre尾与it头相交
            }else if(pre_start <= it_start){
                fout << "pre尾与it头相交" << endl;
                it->data = (pre->data).substr(0, it_start-pre_start) + it->data;
                it->index = pre_start;
                _unassembled_bytes_size = _unassembled_bytes_size - (pre_end - it_start + 1);
            
            // pre头和it尾相交
            }else if(pre_start >= it_start){
                fout << "pre头和it尾相交" << endl;
                it->data = (it->data).substr(0, pre_start-it_start) + pre->data;
                _unassembled_bytes_size = _unassembled_bytes_size - (it_end - pre_start + 1);
            }

            // 删除pre节点
            pre = _unassembled_buffer.erase(pre);
            fout << "_unassembled_bytes_size: " << _unassembled_bytes_size <<endl;
            fout << "final it: " << it->data <<endl;
        }
    }
    fout.close();
}