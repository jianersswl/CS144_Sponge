#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
#include <fstream>

ofstream tfout;
 // fout.open("test.log", ios::app);
 // fout.close();

void TCPReceiver::segment_received(const TCPSegment &seg) { 
    // size_t data_length = seg.length_in_sequence_space();
    string data = seg.payload().copy();
    if(seg.header().syn){
        set_isn(seg.header().seqno);
        _syn = true;
    }

    tfout.open("test.log", ios::app);
    tfout << "***************segement*****************" << endl;
    tfout << seg.header().summary() << endl;
    tfout << data << endl;
    
    // seq num to index
    uint64_t index;
    if(seg.header().syn){
        index= unwrap(seg.header().seqno+1, _isn, _reassembler.get_first_unread())-1;
    }else{
        index= unwrap(seg.header().seqno, _isn, _reassembler.get_first_unread())-1;
    }
    tfout << "stream index: " << index << endl; 
    tfout.close();
    // push data into reaasembler
    _reassembler.push_substring(data, index, seg.header().fin);
    if(_reassembler.stream_out().input_ended()){
        _reassembler.add_first_unread();
    }
    tfout.open("test.log", ios::app);
    tfout << "first_unread: " << _reassembler.get_first_unread() << endl;
    tfout << "ackno: " << (ackno().value()) << endl;
    tfout << "unassembled: " << _reassembler.unassembled_bytes() << endl;; 
    tfout << "****************************************" << endl << endl;
    tfout.close();
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    // return _reassembler.get_first_unread() + _syn;
    if(_syn==false){
        return optional<WrappingInt32>{};
    }
    return wrap(_reassembler.get_first_unread()+1, _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.get_first_unread(); }
