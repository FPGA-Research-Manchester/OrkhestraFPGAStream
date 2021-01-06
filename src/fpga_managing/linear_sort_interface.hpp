#pragma once
class LinearSortInterface{
 public:
  virtual ~LinearSortInterface() = default;
  
  virtual void SetStreamParams(int stream_id, int record_size) = 0;
  virtual void StartPrefetchingData() = 0;
};