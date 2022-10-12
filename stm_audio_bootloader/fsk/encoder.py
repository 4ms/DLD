#!/usr/bin/python2.5
#
# Copyright 2013 Olivier Gillet.
# 
# Author: Olivier Gillet (ol.gillet@gmail.com)
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Fsk encoder for converting firmware .bin files into .

import numpy
import optparse
import zlib

from stm_audio_bootloader import audio_stream_writer


class FskEncoder(object):
  
  def __init__(
      self,
      sample_rate=48000,
      pause_period=32,
      one_period=8,
      zero_period=4,
      packet_size=256):
    self._sr = sample_rate
    self._pause_period = pause_period
    self._one_period = one_period
    self._zero_period = zero_period
    self._packet_size = packet_size
    self._state = 1
    
  def _encode(self, symbol_stream):
    symbol_stream = numpy.array(symbol_stream)
    counts = [numpy.sum(symbol_stream == symbol) for symbol in range(3)]
    durations = [self._zero_period, self._one_period, self._pause_period]
    total_length = numpy.dot(durations, counts)
    signal = numpy.zeros((total_length, 1))
    state = self._state
    index = 0
    for symbol in symbol_stream:
      d = durations[symbol]
      signal[index:index + d] = state
      state = -state
      index += d
    assert index == signal.shape[0]
    self._state = state
    return signal
    
  def _code_blank(self, duration):
    num_symbols = int(duration * self._sr / self._pause_period) + 1
    return self._encode([2] * num_symbols)
    
  def _code_packet(self, data):
    assert len(data) <= self._packet_size
    if len(data) != self._packet_size:
      data = data + b'\x00' * (self._packet_size - len(data))

    crc = zlib.crc32(data) & 0xffffffff

    #data = map(ord, data)
    data = list(data)
    crc_bytes = [crc >> 24, (crc >> 16) & 0xff, (crc >> 8) & 0xff, crc & 0xff]
    bytes = [0x55] * 4 + data + crc_bytes
    
    symbol_stream = []
    for byte in bytes:
      mask = 0x80
      for _ in range(0, 8):
        symbol_stream.append(1 if (byte & mask) else 0)
        mask >>= 1
    
    return self._encode(symbol_stream)

  def code(self, data, page_size=1024, blank_duration=0.06):
    yield numpy.zeros((1 * self._sr, 1)).ravel()
    yield self._code_blank(1.0)
    if len(data) % page_size != 0:
      tail = page_size - (len(data) % page_size)
      data += b'\xff' * tail
    
    offset = 0
    remaining_bytes = len(data)
    num_packets_written = 0
    while remaining_bytes:
      size = min(remaining_bytes, self._packet_size)
      yield self._code_packet(data[offset:offset+size])
      num_packets_written += 1
      if num_packets_written == page_size / self._packet_size:
        yield self._code_blank(blank_duration)
        num_packets_written = 0
      remaining_bytes -= size
      offset += size
    yield self._code_blank(3.5)


def main():
  parser = optparse.OptionParser()
  parser.add_option(
      '-s',
      '--sample_rate',
      dest='sample_rate',
      type='int',
      default=48000,
      help='Sample rate in Hz')
  parser.add_option(
      '-b',
      '--pause_period',
      dest='pause_period',
      type='int',
      default=64,
      help='Period (in samples) of a blank symbol')
  parser.add_option(
      '-n',
      '--one_period',
      dest='one_period',
      type='int',
      default=16,
      help='Period (in samples) of a one symbol')
  parser.add_option(
      '-z',
      '--zero_period',
      dest='zero_period',
      type='int',
      default=8,
      help='Period (in samples) of a zero symbol')
  parser.add_option(
      '-p',
      '--packet_size',
      dest='packet_size',
      type='int',
      default=256,
      help='Packet size in bytes')
  parser.add_option(
      '-g',
      '--page_size',
      dest='page_size',
      type='int',
      default=1024,
      help='Flash page size')
  parser.add_option(
      '-k',
      '--blank_duration',
      dest='blank_duration',
      type='int',
      default=60,
      help='Duration of the blank between pages, in ms')
  parser.add_option(
      '-o',
      '--output_file',
      dest='output_file',
      default=None,
      help='Write output file to FILE',
      metavar='FILE')
  
  options, args = parser.parse_args()
  #data = file(args[0], 'rb').read()
  with open(args[0], 'rb') as input_file:
      data = input_file.read()

  if len(args) != 1:
    logging.fatal('Specify one, and only one firmware .bin file!')
    sys.exit(1)

  output_file = options.output_file
  if not output_file:
    if '.bin' in args[0]:
      output_file = args[0].replace('.bin', '.wav')
    else:
      output_file = args[0] + '.wav'

  encoder = FskEncoder(
      options.sample_rate,
      options.pause_period,
      options.one_period,
      options.zero_period,
      options.packet_size)
  writer = audio_stream_writer.AudioStreamWriter(
      output_file,
      options.sample_rate,
      16,
      1)

  blank_duration = options.blank_duration * 0.001
  for block in encoder.code(data, options.page_size, blank_duration):
    if len(block):
      writer.append(block)
  writer.close()


if __name__ == '__main__':
  main()
