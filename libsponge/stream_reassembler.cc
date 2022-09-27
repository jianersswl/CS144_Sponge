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
_unassembled_buffer(),
_is_eof(false),
_eof_index(0),
_first_unread(0),
_first_unaccept(capacity),
_unassembled_bytes(0),
_output(capacity), 
_capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _first_unaccept = _first_unread + (_capacity - _output.buffer_size());
    if(eof){
        _is_eof = true;
        if(data.length()==0){
            _eof_index = index;
        }else{
            _eof_index = index + data.length()-1;
        }
    }

    // fout.open("test.log", ios::app);
    // fout << "*********************push_substring**********************" << endl;
    // fout << "substring: " << data << endl;
    // fout << "_first_unread: " << _first_unread << endl;
    // fout << "_first_unaccept: " << _first_unaccept << endl;
    // fout.close();

    Tuple node = {data, index, max(index+1, index + data.length())};
    if(check_overflow(node)){
        // _unassembled_buffer.push_front(node);

        // fout.open("test.log", ios::app);
        // fout << "befor_merge: ";
        // for(list<Tuple>::iterator it = _unassembled_buffer.begin(); it!=_unassembled_buffer.end(); ++it){
            // fout << it->data;
        // }
        // fout << endl;
        // fout.close();

        // merge list
        merge(node);

        // fout.open("test.log", ios::app);
        // fout << "after_merge: ";
        // for(list<Tuple>::iterator it = _unassembled_buffer.begin(); it!=_unassembled_buffer.end(); ++it){
        //     fout << it->data;
        // }
        // fout << endl;
        // fout.close();

        //write bytes into Bytestream
        write_bytes();

        // fout.open("test.log", ios::app);
        // fout << "after_write: ";
        // for(list<Tuple>::iterator it = _unassembled_buffer.begin(); it!=_unassembled_buffer.end(); ++it){
        //     fout << it->data;
        // }
        // fout << endl;
        // fout.close();

    }
    // fout.open("test.log", ios::app);
    // fout << "*********************************************************" << endl;
    // fout.close();
}

// 检查头尾是否符合merge的条件
bool StreamReassembler::check_overflow(Tuple &node) {
    if(node.end_index <= _first_unread || node.start_index >= _first_unaccept){
        return false;
    }else{
        if(node.start_index < _first_unread){
            node.data = node.data.substr(_first_unread - node.start_index, node.end_index - _first_unread);
            node.start_index = _first_unread;
        }
        if(node.end_index > _first_unaccept){
            node.data = node.data.substr(0, _first_unaccept - node.start_index);
            node.end_index = _first_unaccept;
        }
    }
    return true;
}

// merge buffer 保证string队列正确性
void StreamReassembler::merge(Tuple &node) { 
    
    // 先摘下来
    // Tuple node = _unassembled_buffer.front();
    // _unassembled_buffer.pop_front();

    // fout.open("test.log", ios::app);
    // fout << "!!!!!!!!!!!!!!!!!!!!!!!!!!merge!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    // fout << "head: " << node.data << endl;

    // 找到第一个可能有交叉的节点
    list<Tuple>::iterator first_cross = _unassembled_buffer.begin();
    for(; first_cross!=_unassembled_buffer.end(); ++first_cross){
        if(first_cross->end_index >= node.start_index){
            break;
        }
    }

    // 空字符算1bytes吗?
    _unassembled_bytes += node.data.length();
    // 没有交叉点直接插入队尾
    if(first_cross == _unassembled_buffer.end()){
        // fout << "insert into th end" << endl;
        _unassembled_buffer.push_back(node);
    }else{
        list<Tuple>::iterator pre = _unassembled_buffer.insert(first_cross, node);
        list<Tuple>::iterator next = first_cross;
        // fout << "pre: " << pre->data << " " << pre->start_index << " " << pre->end_index << endl;
        // fout << "next: " << next->data << " " << next->start_index << " " << next->end_index << endl;
        while(next!=_unassembled_buffer.end()){
            // next已经不可merge
            if(pre->end_index < next->start_index) {
                break;
            }
            // next contains pre
            if(next->start_index <= pre->start_index && next->end_index >= pre->end_index){
                // fout << "next contains pre" << endl;
                _unassembled_bytes -= pre->data.length();
            } 
            // pre contains next 
            else if(pre->start_index <= next->start_index && pre->end_index >= next->end_index){
                // fout << "pre contains next" << endl;

                _unassembled_bytes -= next->data.length();
                next->data = pre->data;
                next->start_index = pre->start_index;
                next->end_index = pre->end_index;

            }
            // pre at left
            else if(pre->start_index < next->start_index){
                // fout << "pre at left" << endl;
                
                _unassembled_bytes -= (pre->end_index - next->start_index);
                next->data = pre->data + (next->data).substr(pre->end_index - next->start_index, next->end_index - pre->end_index);
                next->start_index = pre->start_index;
            }
            // next at left
            else if(next->start_index < pre->start_index){
                // fout << "next at left" << endl;

                _unassembled_bytes -= (next->end_index - pre->start_index);
                next->data = next->data + (pre->data).substr(next->end_index - pre->start_index, pre->end_index - next->end_index);
                next->end_index = pre->end_index;
            }
            _unassembled_buffer.erase(pre);
            pre = next;
            next++;
        }
    }
    // fout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
    // fout.close();
}

// 写入Bytestream
void StreamReassembler::write_bytes() {
    // 先摘下来
    Tuple node = _unassembled_buffer.front();

    if(node.start_index == _first_unread){
        _unassembled_buffer.pop_front();

        _output.write(node.data);
        _first_unread = node.end_index;
        _unassembled_bytes -= node.data.length();

        if(_is_eof == true && node.end_index-1 == _eof_index){
            _output.end_input();
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_buffer.empty(); }
