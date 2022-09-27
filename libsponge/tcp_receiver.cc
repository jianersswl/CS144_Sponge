#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
#include <fstream>

ofstream fout;

void TCPReceiver::segment_received(const TCPSegment &seg) { 
    
    WrappingInt32 seqno = seg.header().seqno;
    size_t seq_len = seg.length_in_sequence_space();
    string data = seg.payload().copy();

    // fout.open("test.log", ios::app);
    // fout << "segment: " << seg.payload().copy() << endl;
    // fout << "seqno: " << seqno << endl;
    

    if(seg.header().syn && !seg.header().fin){
        _SYN = true;
        _isn = seqno;
        seq_len--;

        if(seq_len>0){
            uint64_t absno = unwrap(seqno+1, _isn, _ackno);
            _reassembler.push_substring(data, absno-1, seg.header().fin);
        }
    }else if(!seg.header().syn && seg.header().fin){
        seq_len--;
        if(_SYN){
            uint64_t absno = unwrap(seqno, _isn, _ackno);
            if(seq_len > 0){
                _reassembler.push_substring(data, absno-1, false);
                _reassembler.push_substring("", absno-1+seq_len, true);
            }else{
                _reassembler.push_substring(data, absno-1, true);
            }
            
        }
    }else if(seg.header().syn && seg.header().fin){
        seq_len--;
        seq_len--;
        _SYN = true;
        _isn = seqno;
        
        uint64_t absno = unwrap(seqno+1, _isn, _ackno);
        if(seq_len > 0){
            _reassembler.push_substring(data, absno-1, false);
            _reassembler.push_substring("", absno-1+seq_len, true);
        }else{
            _reassembler.push_substring(data, absno-1, true);
        }
    }else{
        if(_SYN){
            uint64_t absno = unwrap(seqno, _isn, _ackno);
            _reassembler.push_substring(data, absno-1, seg.header().fin);
        }
    }
    // fout.close();
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(_SYN){
        return wrap(_reassembler.get_first_unread()+1, _isn);
    }
    return nullopt;
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
// size_t TCPReceiver::window_size() const { return _reassembler.get_first_unaccept() - _reassembler.get_first_unread(); }
