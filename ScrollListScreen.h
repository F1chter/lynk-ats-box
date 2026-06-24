class ScrollListScreen {
public:
  typedef void (*DrawItemFn)(uint8_t index);

  ScrollListScreen(uint8_t visibleItems, uint8_t itemCount)
    : _visibleItems(visibleItems),
      _itemCount(itemCount) {
    recalculate();
  }

  void setItemCount(uint8_t count) {
    _itemCount = count;
    recalculate();
  }

  bool scrollUp() {
    if (_shift > 0) {
      --_shift;
      return true;
    }
    return false;
  }

  bool scrollDown() {
    if (_shift < _maxShift) {
      ++_shift;
      return true;
    }
    return false;
  }

  void reset() {
    _shift = 0;
  }

  bool canScrollUp() const {
    return _shift > 0;
  }

  bool canScrollDown() const {
    return _shift < _maxShift;
  }

  bool isOnePageList() const {
    return _itemCount <= _visibleItems;
  }

  void draw(DrawItemFn drawItem) const {
    uint8_t end = _shift + _visibleItems;
    if (end > _itemCount)
      end = _itemCount;

    for (uint8_t i = _shift; i < end; ++i) {
      drawItem(i);
    }
  }

  uint8_t getShift() const {
    return _shift;
  }

  void decreaseShiftToShow(uint8_t idx) {
    while(!isVisible(idx) && _shift > 0) _shift--;
  }

  void increaseShiftToShow(uint8_t idx) {
    while(!isVisible(idx) && _shift < _maxShift) _shift++;
  }

private:
  void recalculate() {
    _maxShift = (_itemCount > _visibleItems)
                  ? (1 + _itemCount - _visibleItems)
                  : 0;
    Serial.print("_maxShift");
    Serial.println(_maxShift);
    if (_shift > _maxShift)
      _shift = _maxShift;
  }

  bool isVisible(uint8_t idx) {
    return idx >= _shift && idx < _shift + _visibleItems;
  }

  uint8_t _visibleItems;
  uint8_t _itemCount;
  uint8_t _shift = 0;
  uint8_t _maxShift = 0;
};