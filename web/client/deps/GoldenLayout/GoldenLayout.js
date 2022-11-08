class ExternalError extends Error {
  constructor(type, message) {
    super(message);
    this.type = type;
  }
}
class ConfigurationError extends ExternalError {
  constructor(message, node) {
    super("Configuration", message);
    this.node = node;
  }
}
class PopoutBlockedError extends ExternalError {
  constructor(message) {
    super("PopoutBlocked", message);
  }
}
class ApiError extends ExternalError {
  constructor(message) {
    super("API", message);
  }
}
class BindError extends ExternalError {
  constructor(message) {
    super("Bind", message);
  }
}
class InternalError extends Error {
  constructor(type, code, message) {
    super(`${type}: ${code}${message === void 0 ? "" : ": " + message}`);
  }
}
class AssertError extends InternalError {
  constructor(code, message) {
    super("Assert", code, message);
  }
}
class UnreachableCaseError extends InternalError {
  constructor(code, variableValue, message) {
    super("UnreachableCase", code, `${variableValue}${message === void 0 ? "" : ": " + message}`);
  }
}
class UnexpectedNullError extends InternalError {
  constructor(code, message) {
    super("UnexpectedNull", code, message);
  }
}
class UnexpectedUndefinedError extends InternalError {
  constructor(code, message) {
    super("UnexpectedUndefined", code, message);
  }
}
var WidthOrHeightPropertyName;
(function(WidthOrHeightPropertyName2) {
  WidthOrHeightPropertyName2.width = "width";
  WidthOrHeightPropertyName2.height = "height";
})(WidthOrHeightPropertyName || (WidthOrHeightPropertyName = {}));
var Side;
(function(Side2) {
  Side2.top = "top";
  Side2.left = "left";
  Side2.right = "right";
  Side2.bottom = "bottom";
})(Side || (Side = {}));
var LogicalZIndex;
(function(LogicalZIndex2) {
  LogicalZIndex2.base = "base";
  LogicalZIndex2.drag = "drag";
  LogicalZIndex2.stackMaximised = "stackMaximised";
})(LogicalZIndex || (LogicalZIndex = {}));
var JsonValue;
(function(JsonValue2) {
  function isJson(value) {
    return isJsonObject(value);
  }
  JsonValue2.isJson = isJson;
  function isJsonObject(value) {
    return !Array.isArray(value) && value !== null && typeof value === "object";
  }
  JsonValue2.isJsonObject = isJsonObject;
})(JsonValue || (JsonValue = {}));
var ItemType;
(function(ItemType2) {
  ItemType2.ground = "ground";
  ItemType2.row = "row";
  ItemType2.column = "column";
  ItemType2.stack = "stack";
  ItemType2.component = "component";
})(ItemType || (ItemType = {}));
var ResponsiveMode;
(function(ResponsiveMode2) {
  ResponsiveMode2.none = "none";
  ResponsiveMode2.always = "always";
  ResponsiveMode2.onload = "onload";
})(ResponsiveMode || (ResponsiveMode = {}));
var ConfigMinifier;
(function(ConfigMinifier2) {
  const keys = [
    "settings",
    "hasHeaders",
    "constrainDragToContainer",
    "selectionEnabled",
    "dimensions",
    "borderWidth",
    "minItemHeight",
    "minItemWidth",
    "headerHeight",
    "dragProxyWidth",
    "dragProxyHeight",
    "labels",
    "close",
    "maximise",
    "minimise",
    "popout",
    "content",
    "componentType",
    "componentState",
    "id",
    "width",
    "type",
    "height",
    "isClosable",
    "title",
    "popoutWholeStack",
    "openPopouts",
    "parentId",
    "activeItemIndex",
    "reorderEnabled",
    "borderGrabWidth"
  ];
  const values = [
    true,
    false,
    "row",
    "column",
    "stack",
    "component",
    "close",
    "maximise",
    "minimise",
    "open in new window"
  ];
  function checkInitialise() {
    if (keys.length > 36) {
      throw new Error("Too many keys in config minifier map");
    }
  }
  ConfigMinifier2.checkInitialise = checkInitialise;
  function translateObject(from, minify) {
    const to = {};
    for (const key in from) {
      if (from.hasOwnProperty(key)) {
        let translatedKey;
        if (minify) {
          translatedKey = minifyKey(key);
        } else {
          translatedKey = unminifyKey(key);
        }
        const fromValue = from[key];
        to[translatedKey] = translateValue(fromValue, minify);
      }
    }
    return to;
  }
  ConfigMinifier2.translateObject = translateObject;
  function translateArray(from, minify) {
    const length = from.length;
    const to = new Array(length);
    for (let i = 0; i < length; i++) {
      const fromValue = from[i];
      to[i] = translateValue(fromValue, minify);
    }
    return to;
  }
  function translateValue(from, minify) {
    if (typeof from === "object") {
      if (from === null) {
        return null;
      } else {
        if (Array.isArray(from)) {
          return translateArray(from, minify);
        } else {
          return translateObject(from, minify);
        }
      }
    } else {
      if (minify) {
        return minifyValue(from);
      } else {
        return unminifyValue(from);
      }
    }
  }
  function minifyKey(value) {
    if (typeof value === "string" && value.length === 1) {
      return "___" + value;
    }
    const index = indexOfKey(value);
    if (index === -1) {
      return value;
    } else {
      return index.toString(36);
    }
  }
  function unminifyKey(key) {
    if (key.length === 1) {
      return keys[parseInt(key, 36)];
    }
    if (key.substr(0, 3) === "___") {
      return key[3];
    }
    return key;
  }
  function minifyValue(value) {
    if (typeof value === "string" && value.length === 1) {
      return "___" + value;
    }
    const index = indexOfValue(value);
    if (index === -1) {
      return value;
    } else {
      return index.toString(36);
    }
  }
  function unminifyValue(value) {
    if (typeof value === "string" && value.length === 1) {
      return values[parseInt(value, 36)];
    }
    if (typeof value === "string" && value.substr(0, 3) === "___") {
      return value[3];
    }
    return value;
  }
  function indexOfKey(key) {
    for (let i = 0; i < keys.length; i++) {
      if (keys[i] === key) {
        return i;
      }
    }
    return -1;
  }
  function indexOfValue(value) {
    for (let i = 0; i < values.length; i++) {
      if (values[i] === value) {
        return i;
      }
    }
    return -1;
  }
})(ConfigMinifier || (ConfigMinifier = {}));
function numberToPixels(value) {
  return value.toString(10) + "px";
}
function pixelsToNumber(value) {
  const numberStr = value.replace("px", "");
  return parseFloat(numberStr);
}
function getElementWidth(element) {
  return element.offsetWidth;
}
function setElementWidth(element, width) {
  const widthAsPixels = numberToPixels(width);
  element.style.width = widthAsPixels;
}
function getElementHeight(element) {
  return element.offsetHeight;
}
function setElementHeight(element, height) {
  const heightAsPixels = numberToPixels(height);
  element.style.height = heightAsPixels;
}
function getElementWidthAndHeight(element) {
  return {
    width: element.offsetWidth,
    height: element.offsetHeight
  };
}
function setElementDisplayVisibility(element, visible) {
  if (visible) {
    element.style.display = "";
  } else {
    element.style.display = "none";
  }
}
function ensureElementPositionAbsolute(element) {
  const absolutePosition = "absolute";
  if (element.style.position !== absolutePosition) {
    element.style.position = absolutePosition;
  }
}
function deepExtend(target, obj) {
  if (obj !== void 0) {
    for (const key in obj) {
      if (obj.hasOwnProperty(key)) {
        const value = obj[key];
        const existingTarget = target[key];
        target[key] = deepExtendValue(existingTarget, value);
      }
    }
  }
  return target;
}
function deepExtendValue(existingTarget, value) {
  if (typeof value !== "object") {
    return value;
  } else {
    if (Array.isArray(value)) {
      const length = value.length;
      const targetArray = new Array(length);
      for (let i = 0; i < length; i++) {
        const element = value[i];
        targetArray[i] = deepExtendValue({}, element);
      }
      return targetArray;
    } else {
      if (value === null) {
        return null;
      } else {
        const valueObj = value;
        if (existingTarget === void 0) {
          return deepExtend({}, valueObj);
        } else {
          if (typeof existingTarget !== "object") {
            return deepExtend({}, valueObj);
          } else {
            if (Array.isArray(existingTarget)) {
              return deepExtend({}, valueObj);
            } else {
              if (existingTarget === null) {
                return deepExtend({}, valueObj);
              } else {
                const existingTargetObj = existingTarget;
                return deepExtend(existingTargetObj, valueObj);
              }
            }
          }
        }
      }
    }
  }
}
function removeFromArray(item, array) {
  const index = array.indexOf(item);
  if (index === -1) {
    throw new Error("Can't remove item from array. Item is not in the array");
  }
  array.splice(index, 1);
}
function getUniqueId() {
  return (Math.random() * 1e15).toString(36).replace(".", "");
}
var ResolvedItemConfig;
(function(ResolvedItemConfig2) {
  ResolvedItemConfig2.defaults = {
    type: ItemType.ground,
    content: [],
    width: 50,
    minWidth: 0,
    height: 50,
    minHeight: 0,
    id: "",
    isClosable: true
  };
  function createCopy(original, content) {
    switch (original.type) {
      case ItemType.ground:
      case ItemType.row:
      case ItemType.column:
        return ResolvedRowOrColumnItemConfig.createCopy(original, content);
      case ItemType.stack:
        return ResolvedStackItemConfig.createCopy(original, content);
      case ItemType.component:
        return ResolvedComponentItemConfig.createCopy(original);
      default:
        throw new UnreachableCaseError("CICC91354", original.type, "Invalid Config Item type specified");
    }
  }
  ResolvedItemConfig2.createCopy = createCopy;
  function createDefault(type) {
    switch (type) {
      case ItemType.ground:
        throw new AssertError("CICCDR91562");
      case ItemType.row:
      case ItemType.column:
        return ResolvedRowOrColumnItemConfig.createDefault(type);
      case ItemType.stack:
        return ResolvedStackItemConfig.createDefault();
      case ItemType.component:
        return ResolvedComponentItemConfig.createDefault();
      default:
        throw new UnreachableCaseError("CICCDD91563", type, "Invalid Config Item type specified");
    }
  }
  ResolvedItemConfig2.createDefault = createDefault;
  function isComponentItem(itemConfig) {
    return itemConfig.type === ItemType.component;
  }
  ResolvedItemConfig2.isComponentItem = isComponentItem;
  function isStackItem(itemConfig) {
    return itemConfig.type === ItemType.stack;
  }
  ResolvedItemConfig2.isStackItem = isStackItem;
  function isGroundItem(itemConfig) {
    return itemConfig.type === ItemType.ground;
  }
  ResolvedItemConfig2.isGroundItem = isGroundItem;
})(ResolvedItemConfig || (ResolvedItemConfig = {}));
var ResolvedHeaderedItemConfig;
(function(ResolvedHeaderedItemConfig2) {
  ResolvedHeaderedItemConfig2.defaultMaximised = false;
  (function(Header2) {
    function createCopy(original, show) {
      if (original === void 0) {
        return void 0;
      } else {
        return {
          show: show !== null && show !== void 0 ? show : original.show,
          popout: original.popout,
          close: original.close,
          maximise: original.maximise,
          minimise: original.minimise,
          tabDropdown: original.tabDropdown
        };
      }
    }
    Header2.createCopy = createCopy;
  })(ResolvedHeaderedItemConfig2.Header || (ResolvedHeaderedItemConfig2.Header = {}));
})(ResolvedHeaderedItemConfig || (ResolvedHeaderedItemConfig = {}));
var ResolvedStackItemConfig;
(function(ResolvedStackItemConfig2) {
  ResolvedStackItemConfig2.defaultActiveItemIndex = 0;
  function createCopy(original, content) {
    const result = {
      type: original.type,
      content: content !== void 0 ? copyContent(content) : copyContent(original.content),
      width: original.width,
      minWidth: original.minWidth,
      height: original.height,
      minHeight: original.minHeight,
      id: original.id,
      maximised: original.maximised,
      isClosable: original.isClosable,
      activeItemIndex: original.activeItemIndex,
      header: ResolvedHeaderedItemConfig.Header.createCopy(original.header)
    };
    return result;
  }
  ResolvedStackItemConfig2.createCopy = createCopy;
  function copyContent(original) {
    const count = original.length;
    const result = new Array(count);
    for (let i = 0; i < count; i++) {
      result[i] = ResolvedItemConfig.createCopy(original[i]);
    }
    return result;
  }
  ResolvedStackItemConfig2.copyContent = copyContent;
  function createDefault() {
    const result = {
      type: ItemType.stack,
      content: [],
      width: ResolvedItemConfig.defaults.width,
      minWidth: ResolvedItemConfig.defaults.minWidth,
      height: ResolvedItemConfig.defaults.height,
      minHeight: ResolvedItemConfig.defaults.minHeight,
      id: ResolvedItemConfig.defaults.id,
      maximised: ResolvedHeaderedItemConfig.defaultMaximised,
      isClosable: ResolvedItemConfig.defaults.isClosable,
      activeItemIndex: ResolvedStackItemConfig2.defaultActiveItemIndex,
      header: void 0
    };
    return result;
  }
  ResolvedStackItemConfig2.createDefault = createDefault;
})(ResolvedStackItemConfig || (ResolvedStackItemConfig = {}));
var ResolvedComponentItemConfig;
(function(ResolvedComponentItemConfig2) {
  ResolvedComponentItemConfig2.defaultReorderEnabled = true;
  function resolveComponentTypeName(itemConfig) {
    const componentType = itemConfig.componentType;
    if (typeof componentType === "string") {
      return componentType;
    } else {
      return void 0;
    }
  }
  ResolvedComponentItemConfig2.resolveComponentTypeName = resolveComponentTypeName;
  function createCopy(original) {
    const result = {
      type: original.type,
      content: [],
      width: original.width,
      minWidth: original.minWidth,
      height: original.height,
      minHeight: original.minHeight,
      id: original.id,
      maximised: original.maximised,
      isClosable: original.isClosable,
      reorderEnabled: original.reorderEnabled,
      title: original.title,
      header: ResolvedHeaderedItemConfig.Header.createCopy(original.header),
      componentType: original.componentType,
      componentState: deepExtendValue(void 0, original.componentState)
    };
    return result;
  }
  ResolvedComponentItemConfig2.createCopy = createCopy;
  function createDefault(componentType = "", componentState, title = "") {
    const result = {
      type: ItemType.component,
      content: [],
      width: ResolvedItemConfig.defaults.width,
      minWidth: ResolvedItemConfig.defaults.minWidth,
      height: ResolvedItemConfig.defaults.height,
      minHeight: ResolvedItemConfig.defaults.minHeight,
      id: ResolvedItemConfig.defaults.id,
      maximised: ResolvedHeaderedItemConfig.defaultMaximised,
      isClosable: ResolvedItemConfig.defaults.isClosable,
      reorderEnabled: ResolvedComponentItemConfig2.defaultReorderEnabled,
      title,
      header: void 0,
      componentType,
      componentState
    };
    return result;
  }
  ResolvedComponentItemConfig2.createDefault = createDefault;
  function copyComponentType(componentType) {
    return deepExtendValue({}, componentType);
  }
  ResolvedComponentItemConfig2.copyComponentType = copyComponentType;
})(ResolvedComponentItemConfig || (ResolvedComponentItemConfig = {}));
var ResolvedRowOrColumnItemConfig;
(function(ResolvedRowOrColumnItemConfig2) {
  function isChildItemConfig(itemConfig) {
    switch (itemConfig.type) {
      case ItemType.row:
      case ItemType.column:
      case ItemType.stack:
      case ItemType.component:
        return true;
      case ItemType.ground:
        return false;
      default:
        throw new UnreachableCaseError("CROCOSPCICIC13687", itemConfig.type);
    }
  }
  ResolvedRowOrColumnItemConfig2.isChildItemConfig = isChildItemConfig;
  function createCopy(original, content) {
    const result = {
      type: original.type,
      content: content !== void 0 ? copyContent(content) : copyContent(original.content),
      width: original.width,
      minWidth: original.minWidth,
      height: original.height,
      minHeight: original.minHeight,
      id: original.id,
      isClosable: original.isClosable
    };
    return result;
  }
  ResolvedRowOrColumnItemConfig2.createCopy = createCopy;
  function copyContent(original) {
    const count = original.length;
    const result = new Array(count);
    for (let i = 0; i < count; i++) {
      result[i] = ResolvedItemConfig.createCopy(original[i]);
    }
    return result;
  }
  ResolvedRowOrColumnItemConfig2.copyContent = copyContent;
  function createDefault(type) {
    const result = {
      type,
      content: [],
      width: ResolvedItemConfig.defaults.width,
      minWidth: ResolvedItemConfig.defaults.minWidth,
      height: ResolvedItemConfig.defaults.height,
      minHeight: ResolvedItemConfig.defaults.minHeight,
      id: ResolvedItemConfig.defaults.id,
      isClosable: ResolvedItemConfig.defaults.isClosable
    };
    return result;
  }
  ResolvedRowOrColumnItemConfig2.createDefault = createDefault;
})(ResolvedRowOrColumnItemConfig || (ResolvedRowOrColumnItemConfig = {}));
var ResolvedRootItemConfig;
(function(ResolvedRootItemConfig2) {
  function createCopy(config) {
    return ResolvedItemConfig.createCopy(config);
  }
  ResolvedRootItemConfig2.createCopy = createCopy;
  function isRootItemConfig(itemConfig) {
    switch (itemConfig.type) {
      case ItemType.row:
      case ItemType.column:
      case ItemType.stack:
      case ItemType.component:
        return true;
      case ItemType.ground:
        return false;
      default:
        throw new UnreachableCaseError("CROCOSPCICIC13687", itemConfig.type);
    }
  }
  ResolvedRootItemConfig2.isRootItemConfig = isRootItemConfig;
})(ResolvedRootItemConfig || (ResolvedRootItemConfig = {}));
var ResolvedGroundItemConfig;
(function(ResolvedGroundItemConfig2) {
  function create(rootItemConfig) {
    const content = rootItemConfig === void 0 ? [] : [rootItemConfig];
    return {
      type: ItemType.ground,
      content,
      width: 100,
      minWidth: 0,
      height: 100,
      minHeight: 0,
      id: "",
      isClosable: false,
      title: "",
      reorderEnabled: false
    };
  }
  ResolvedGroundItemConfig2.create = create;
})(ResolvedGroundItemConfig || (ResolvedGroundItemConfig = {}));
var ResolvedLayoutConfig;
(function(ResolvedLayoutConfig2) {
  (function(Settings) {
    Settings.defaults = {
      constrainDragToContainer: true,
      reorderEnabled: true,
      popoutWholeStack: false,
      blockedPopoutsThrowError: true,
      closePopoutsOnUnload: true,
      responsiveMode: ResponsiveMode.none,
      tabOverlapAllowance: 0,
      reorderOnTabMenuClick: true,
      tabControlOffset: 10,
      popInOnClose: false
    };
    function createCopy2(original) {
      return {
        constrainDragToContainer: original.constrainDragToContainer,
        reorderEnabled: original.reorderEnabled,
        popoutWholeStack: original.popoutWholeStack,
        blockedPopoutsThrowError: original.blockedPopoutsThrowError,
        closePopoutsOnUnload: original.closePopoutsOnUnload,
        responsiveMode: original.responsiveMode,
        tabOverlapAllowance: original.tabOverlapAllowance,
        reorderOnTabMenuClick: original.reorderOnTabMenuClick,
        tabControlOffset: original.tabControlOffset,
        popInOnClose: original.popInOnClose
      };
    }
    Settings.createCopy = createCopy2;
  })(ResolvedLayoutConfig2.Settings || (ResolvedLayoutConfig2.Settings = {}));
  (function(Dimensions) {
    function createCopy2(original) {
      return {
        borderWidth: original.borderWidth,
        borderGrabWidth: original.borderGrabWidth,
        minItemHeight: original.minItemHeight,
        minItemWidth: original.minItemWidth,
        headerHeight: original.headerHeight,
        dragProxyWidth: original.dragProxyWidth,
        dragProxyHeight: original.dragProxyHeight
      };
    }
    Dimensions.createCopy = createCopy2;
    Dimensions.defaults = {
      borderWidth: 5,
      borderGrabWidth: 5,
      minItemHeight: 10,
      minItemWidth: 10,
      headerHeight: 20,
      dragProxyWidth: 300,
      dragProxyHeight: 200
    };
  })(ResolvedLayoutConfig2.Dimensions || (ResolvedLayoutConfig2.Dimensions = {}));
  (function(Header2) {
    function createCopy2(original) {
      return {
        show: original.show,
        popout: original.popout,
        dock: original.dock,
        close: original.close,
        maximise: original.maximise,
        minimise: original.minimise,
        tabDropdown: original.tabDropdown
      };
    }
    Header2.createCopy = createCopy2;
    Header2.defaults = {
      show: Side.top,
      popout: "open in new window",
      dock: "dock",
      maximise: "maximise",
      minimise: "minimise",
      close: "close",
      tabDropdown: "additional tabs"
    };
  })(ResolvedLayoutConfig2.Header || (ResolvedLayoutConfig2.Header = {}));
  function isPopout(config) {
    return "parentId" in config;
  }
  ResolvedLayoutConfig2.isPopout = isPopout;
  function createDefault() {
    const result = {
      root: void 0,
      openPopouts: [],
      dimensions: ResolvedLayoutConfig2.Dimensions.defaults,
      settings: ResolvedLayoutConfig2.Settings.defaults,
      header: ResolvedLayoutConfig2.Header.defaults,
      resolved: true
    };
    return result;
  }
  ResolvedLayoutConfig2.createDefault = createDefault;
  function createCopy(config) {
    if (isPopout(config)) {
      return ResolvedPopoutLayoutConfig.createCopy(config);
    } else {
      const result = {
        root: config.root === void 0 ? void 0 : ResolvedRootItemConfig.createCopy(config.root),
        openPopouts: ResolvedLayoutConfig2.copyOpenPopouts(config.openPopouts),
        settings: ResolvedLayoutConfig2.Settings.createCopy(config.settings),
        dimensions: ResolvedLayoutConfig2.Dimensions.createCopy(config.dimensions),
        header: ResolvedLayoutConfig2.Header.createCopy(config.header),
        resolved: config.resolved
      };
      return result;
    }
  }
  ResolvedLayoutConfig2.createCopy = createCopy;
  function copyOpenPopouts(original) {
    const count = original.length;
    const result = new Array(count);
    for (let i = 0; i < count; i++) {
      result[i] = ResolvedPopoutLayoutConfig.createCopy(original[i]);
    }
    return result;
  }
  ResolvedLayoutConfig2.copyOpenPopouts = copyOpenPopouts;
  function minifyConfig(layoutConfig) {
    return ConfigMinifier.translateObject(layoutConfig, true);
  }
  ResolvedLayoutConfig2.minifyConfig = minifyConfig;
  function unminifyConfig(minifiedConfig) {
    return ConfigMinifier.translateObject(minifiedConfig, false);
  }
  ResolvedLayoutConfig2.unminifyConfig = unminifyConfig;
})(ResolvedLayoutConfig || (ResolvedLayoutConfig = {}));
var ResolvedPopoutLayoutConfig;
(function(ResolvedPopoutLayoutConfig2) {
  (function(Window) {
    function createCopy2(original) {
      return {
        width: original.width,
        height: original.height,
        left: original.left,
        top: original.top
      };
    }
    Window.createCopy = createCopy2;
    Window.defaults = {
      width: null,
      height: null,
      left: null,
      top: null
    };
  })(ResolvedPopoutLayoutConfig2.Window || (ResolvedPopoutLayoutConfig2.Window = {}));
  function createCopy(original) {
    const result = {
      root: original.root === void 0 ? void 0 : ResolvedRootItemConfig.createCopy(original.root),
      openPopouts: ResolvedLayoutConfig.copyOpenPopouts(original.openPopouts),
      settings: ResolvedLayoutConfig.Settings.createCopy(original.settings),
      dimensions: ResolvedLayoutConfig.Dimensions.createCopy(original.dimensions),
      header: ResolvedLayoutConfig.Header.createCopy(original.header),
      parentId: original.parentId,
      indexInParent: original.indexInParent,
      window: ResolvedPopoutLayoutConfig2.Window.createCopy(original.window),
      resolved: original.resolved
    };
    return result;
  }
  ResolvedPopoutLayoutConfig2.createCopy = createCopy;
})(ResolvedPopoutLayoutConfig || (ResolvedPopoutLayoutConfig = {}));
var ItemConfig;
(function(ItemConfig2) {
  function resolve(itemConfig) {
    switch (itemConfig.type) {
      case ItemType.ground:
        throw new ConfigurationError("ItemConfig cannot specify type ground", JSON.stringify(itemConfig));
      case ItemType.row:
      case ItemType.column:
        return RowOrColumnItemConfig.resolve(itemConfig);
      case ItemType.stack:
        return StackItemConfig.resolve(itemConfig);
      case ItemType.component:
        return ComponentItemConfig.resolve(itemConfig);
      default:
        throw new UnreachableCaseError("UCUICR55499", itemConfig.type);
    }
  }
  ItemConfig2.resolve = resolve;
  function resolveContent(content) {
    if (content === void 0) {
      return [];
    } else {
      const count = content.length;
      const result = new Array(count);
      for (let i = 0; i < count; i++) {
        result[i] = ItemConfig2.resolve(content[i]);
      }
      return result;
    }
  }
  ItemConfig2.resolveContent = resolveContent;
  function resolveId(id) {
    if (id === void 0) {
      return ResolvedItemConfig.defaults.id;
    } else {
      if (Array.isArray(id)) {
        if (id.length === 0) {
          return ResolvedItemConfig.defaults.id;
        } else {
          return id[0];
        }
      } else {
        return id;
      }
    }
  }
  ItemConfig2.resolveId = resolveId;
  function isGround(config) {
    return config.type === ItemType.ground;
  }
  ItemConfig2.isGround = isGround;
  function isRow(config) {
    return config.type === ItemType.row;
  }
  ItemConfig2.isRow = isRow;
  function isColumn(config) {
    return config.type === ItemType.column;
  }
  ItemConfig2.isColumn = isColumn;
  function isStack(config) {
    return config.type === ItemType.stack;
  }
  ItemConfig2.isStack = isStack;
  function isComponent(config) {
    return config.type === ItemType.component;
  }
  ItemConfig2.isComponent = isComponent;
})(ItemConfig || (ItemConfig = {}));
var HeaderedItemConfig;
(function(HeaderedItemConfig2) {
  const legacyMaximisedId = "__glMaximised";
  (function(Header2) {
    function resolve(header, hasHeaders) {
      var _a;
      if (header === void 0 && hasHeaders === void 0) {
        return void 0;
      } else {
        const result = {
          show: (_a = header === null || header === void 0 ? void 0 : header.show) !== null && _a !== void 0 ? _a : hasHeaders === void 0 ? void 0 : hasHeaders ? ResolvedLayoutConfig.Header.defaults.show : false,
          popout: header === null || header === void 0 ? void 0 : header.popout,
          maximise: header === null || header === void 0 ? void 0 : header.maximise,
          close: header === null || header === void 0 ? void 0 : header.close,
          minimise: header === null || header === void 0 ? void 0 : header.minimise,
          tabDropdown: header === null || header === void 0 ? void 0 : header.tabDropdown
        };
        return result;
      }
    }
    Header2.resolve = resolve;
  })(HeaderedItemConfig2.Header || (HeaderedItemConfig2.Header = {}));
  function resolveIdAndMaximised(config) {
    let id;
    let legacyId = config.id;
    let legacyMaximised = false;
    if (legacyId === void 0) {
      id = ResolvedItemConfig.defaults.id;
    } else {
      if (Array.isArray(legacyId)) {
        const idx = legacyId.findIndex((id2) => id2 === legacyMaximisedId);
        if (idx > 0) {
          legacyMaximised = true;
          legacyId = legacyId.splice(idx, 1);
        }
        if (legacyId.length > 0) {
          id = legacyId[0];
        } else {
          id = ResolvedItemConfig.defaults.id;
        }
      } else {
        id = legacyId;
      }
    }
    let maximised;
    if (config.maximised !== void 0) {
      maximised = config.maximised;
    } else {
      maximised = legacyMaximised;
    }
    return {id, maximised};
  }
  HeaderedItemConfig2.resolveIdAndMaximised = resolveIdAndMaximised;
})(HeaderedItemConfig || (HeaderedItemConfig = {}));
var StackItemConfig;
(function(StackItemConfig2) {
  function resolve(itemConfig) {
    var _a, _b, _c, _d, _e, _f;
    const {id, maximised} = HeaderedItemConfig.resolveIdAndMaximised(itemConfig);
    const result = {
      type: ItemType.stack,
      content: resolveContent(itemConfig.content),
      width: (_a = itemConfig.width) !== null && _a !== void 0 ? _a : ResolvedItemConfig.defaults.width,
      minWidth: (_b = itemConfig.minWidth) !== null && _b !== void 0 ? _b : ResolvedItemConfig.defaults.minWidth,
      height: (_c = itemConfig.height) !== null && _c !== void 0 ? _c : ResolvedItemConfig.defaults.height,
      minHeight: (_d = itemConfig.minHeight) !== null && _d !== void 0 ? _d : ResolvedItemConfig.defaults.minHeight,
      id,
      maximised,
      isClosable: (_e = itemConfig.isClosable) !== null && _e !== void 0 ? _e : ResolvedItemConfig.defaults.isClosable,
      activeItemIndex: (_f = itemConfig.activeItemIndex) !== null && _f !== void 0 ? _f : ResolvedStackItemConfig.defaultActiveItemIndex,
      header: HeaderedItemConfig.Header.resolve(itemConfig.header, itemConfig.hasHeaders)
    };
    return result;
  }
  StackItemConfig2.resolve = resolve;
  function resolveContent(content) {
    if (content === void 0) {
      return [];
    } else {
      const count = content.length;
      const result = new Array(count);
      for (let i = 0; i < count; i++) {
        const childItemConfig = content[i];
        const itemConfig = ItemConfig.resolve(childItemConfig);
        if (!ResolvedItemConfig.isComponentItem(itemConfig)) {
          throw new AssertError("UCUSICRC91114", JSON.stringify(itemConfig));
        } else {
          result[i] = itemConfig;
        }
      }
      return result;
    }
  }
  StackItemConfig2.resolveContent = resolveContent;
})(StackItemConfig || (StackItemConfig = {}));
var ComponentItemConfig;
(function(ComponentItemConfig2) {
  function resolve(itemConfig) {
    var _a, _b, _c, _d, _e, _f, _g;
    let componentType = itemConfig.componentType;
    if (componentType === void 0) {
      componentType = itemConfig.componentName;
    }
    if (componentType === void 0) {
      throw new Error("ComponentItemConfig.componentType is undefined");
    } else {
      const {id, maximised} = HeaderedItemConfig.resolveIdAndMaximised(itemConfig);
      let title;
      if (itemConfig.title === void 0 || itemConfig.title === "") {
        title = ComponentItemConfig2.componentTypeToTitle(componentType);
      } else {
        title = itemConfig.title;
      }
      const result = {
        type: itemConfig.type,
        content: [],
        width: (_a = itemConfig.width) !== null && _a !== void 0 ? _a : ResolvedItemConfig.defaults.width,
        minWidth: (_b = itemConfig.minWidth) !== null && _b !== void 0 ? _b : ResolvedItemConfig.defaults.minWidth,
        height: (_c = itemConfig.height) !== null && _c !== void 0 ? _c : ResolvedItemConfig.defaults.height,
        minHeight: (_d = itemConfig.minHeight) !== null && _d !== void 0 ? _d : ResolvedItemConfig.defaults.minHeight,
        id,
        maximised,
        isClosable: (_e = itemConfig.isClosable) !== null && _e !== void 0 ? _e : ResolvedItemConfig.defaults.isClosable,
        reorderEnabled: (_f = itemConfig.reorderEnabled) !== null && _f !== void 0 ? _f : ResolvedComponentItemConfig.defaultReorderEnabled,
        title,
        header: HeaderedItemConfig.Header.resolve(itemConfig.header, itemConfig.hasHeaders),
        componentType,
        componentState: (_g = itemConfig.componentState) !== null && _g !== void 0 ? _g : {}
      };
      return result;
    }
  }
  ComponentItemConfig2.resolve = resolve;
  function componentTypeToTitle(componentType) {
    const componentTypeType = typeof componentType;
    switch (componentTypeType) {
      case "string":
        return componentType;
      case "number":
        return componentType.toString();
      case "boolean":
        return componentType.toString();
      default:
        return "";
    }
  }
  ComponentItemConfig2.componentTypeToTitle = componentTypeToTitle;
})(ComponentItemConfig || (ComponentItemConfig = {}));
var RowOrColumnItemConfig;
(function(RowOrColumnItemConfig2) {
  function isChildItemConfig(itemConfig) {
    switch (itemConfig.type) {
      case ItemType.row:
      case ItemType.column:
      case ItemType.stack:
      case ItemType.component:
        return true;
      case ItemType.ground:
        return false;
      default:
        throw new UnreachableCaseError("UROCOSPCICIC13687", itemConfig.type);
    }
  }
  RowOrColumnItemConfig2.isChildItemConfig = isChildItemConfig;
  function resolve(itemConfig) {
    var _a, _b, _c, _d, _e;
    const result = {
      type: itemConfig.type,
      content: RowOrColumnItemConfig2.resolveContent(itemConfig.content),
      width: (_a = itemConfig.width) !== null && _a !== void 0 ? _a : ResolvedItemConfig.defaults.width,
      minWidth: (_b = itemConfig.width) !== null && _b !== void 0 ? _b : ResolvedItemConfig.defaults.minWidth,
      height: (_c = itemConfig.height) !== null && _c !== void 0 ? _c : ResolvedItemConfig.defaults.height,
      minHeight: (_d = itemConfig.height) !== null && _d !== void 0 ? _d : ResolvedItemConfig.defaults.minHeight,
      id: ItemConfig.resolveId(itemConfig.id),
      isClosable: (_e = itemConfig.isClosable) !== null && _e !== void 0 ? _e : ResolvedItemConfig.defaults.isClosable
    };
    return result;
  }
  RowOrColumnItemConfig2.resolve = resolve;
  function resolveContent(content) {
    if (content === void 0) {
      return [];
    } else {
      const count = content.length;
      const result = new Array(count);
      for (let i = 0; i < count; i++) {
        const childItemConfig = content[i];
        if (!RowOrColumnItemConfig2.isChildItemConfig(childItemConfig)) {
          throw new ConfigurationError("ItemConfig is not Row, Column or Stack", childItemConfig);
        } else {
          const resolvedChildItemConfig = ItemConfig.resolve(childItemConfig);
          if (!ResolvedRowOrColumnItemConfig.isChildItemConfig(resolvedChildItemConfig)) {
            throw new AssertError("UROCOSPIC99512", JSON.stringify(resolvedChildItemConfig));
          } else {
            result[i] = resolvedChildItemConfig;
          }
        }
      }
      return result;
    }
  }
  RowOrColumnItemConfig2.resolveContent = resolveContent;
})(RowOrColumnItemConfig || (RowOrColumnItemConfig = {}));
var RootItemConfig;
(function(RootItemConfig2) {
  function isRootItemConfig(itemConfig) {
    switch (itemConfig.type) {
      case ItemType.row:
      case ItemType.column:
      case ItemType.stack:
      case ItemType.component:
        return true;
      case ItemType.ground:
        return false;
      default:
        throw new UnreachableCaseError("URICIR23687", itemConfig.type);
    }
  }
  RootItemConfig2.isRootItemConfig = isRootItemConfig;
  function resolve(itemConfig) {
    if (itemConfig === void 0) {
      return void 0;
    } else {
      const result = ItemConfig.resolve(itemConfig);
      if (!ResolvedRootItemConfig.isRootItemConfig(result)) {
        throw new ConfigurationError("ItemConfig is not Row, Column or Stack", JSON.stringify(itemConfig));
      } else {
        return result;
      }
    }
  }
  RootItemConfig2.resolve = resolve;
})(RootItemConfig || (RootItemConfig = {}));
var LayoutConfig;
(function(LayoutConfig2) {
  (function(Settings) {
    function resolve2(settings) {
      var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k;
      const result = {
        constrainDragToContainer: (_a = settings === null || settings === void 0 ? void 0 : settings.constrainDragToContainer) !== null && _a !== void 0 ? _a : ResolvedLayoutConfig.Settings.defaults.constrainDragToContainer,
        reorderEnabled: (_b = settings === null || settings === void 0 ? void 0 : settings.reorderEnabled) !== null && _b !== void 0 ? _b : ResolvedLayoutConfig.Settings.defaults.reorderEnabled,
        popoutWholeStack: (_c = settings === null || settings === void 0 ? void 0 : settings.popoutWholeStack) !== null && _c !== void 0 ? _c : ResolvedLayoutConfig.Settings.defaults.popoutWholeStack,
        blockedPopoutsThrowError: (_d = settings === null || settings === void 0 ? void 0 : settings.blockedPopoutsThrowError) !== null && _d !== void 0 ? _d : ResolvedLayoutConfig.Settings.defaults.blockedPopoutsThrowError,
        closePopoutsOnUnload: (_e = settings === null || settings === void 0 ? void 0 : settings.closePopoutsOnUnload) !== null && _e !== void 0 ? _e : ResolvedLayoutConfig.Settings.defaults.closePopoutsOnUnload,
        responsiveMode: (_f = settings === null || settings === void 0 ? void 0 : settings.responsiveMode) !== null && _f !== void 0 ? _f : ResolvedLayoutConfig.Settings.defaults.responsiveMode,
        tabOverlapAllowance: (_g = settings === null || settings === void 0 ? void 0 : settings.tabOverlapAllowance) !== null && _g !== void 0 ? _g : ResolvedLayoutConfig.Settings.defaults.tabOverlapAllowance,
        reorderOnTabMenuClick: (_h = settings === null || settings === void 0 ? void 0 : settings.reorderOnTabMenuClick) !== null && _h !== void 0 ? _h : ResolvedLayoutConfig.Settings.defaults.reorderOnTabMenuClick,
        tabControlOffset: (_j = settings === null || settings === void 0 ? void 0 : settings.tabControlOffset) !== null && _j !== void 0 ? _j : ResolvedLayoutConfig.Settings.defaults.tabControlOffset,
        popInOnClose: (_k = settings === null || settings === void 0 ? void 0 : settings.popInOnClose) !== null && _k !== void 0 ? _k : ResolvedLayoutConfig.Settings.defaults.popInOnClose
      };
      return result;
    }
    Settings.resolve = resolve2;
  })(LayoutConfig2.Settings || (LayoutConfig2.Settings = {}));
  (function(Dimensions) {
    function resolve2(dimensions) {
      var _a, _b, _c, _d, _e, _f, _g;
      const result = {
        borderWidth: (_a = dimensions === null || dimensions === void 0 ? void 0 : dimensions.borderWidth) !== null && _a !== void 0 ? _a : ResolvedLayoutConfig.Dimensions.defaults.borderWidth,
        borderGrabWidth: (_b = dimensions === null || dimensions === void 0 ? void 0 : dimensions.borderGrabWidth) !== null && _b !== void 0 ? _b : ResolvedLayoutConfig.Dimensions.defaults.borderGrabWidth,
        minItemHeight: (_c = dimensions === null || dimensions === void 0 ? void 0 : dimensions.minItemHeight) !== null && _c !== void 0 ? _c : ResolvedLayoutConfig.Dimensions.defaults.minItemHeight,
        minItemWidth: (_d = dimensions === null || dimensions === void 0 ? void 0 : dimensions.minItemWidth) !== null && _d !== void 0 ? _d : ResolvedLayoutConfig.Dimensions.defaults.minItemWidth,
        headerHeight: (_e = dimensions === null || dimensions === void 0 ? void 0 : dimensions.headerHeight) !== null && _e !== void 0 ? _e : ResolvedLayoutConfig.Dimensions.defaults.headerHeight,
        dragProxyWidth: (_f = dimensions === null || dimensions === void 0 ? void 0 : dimensions.dragProxyWidth) !== null && _f !== void 0 ? _f : ResolvedLayoutConfig.Dimensions.defaults.dragProxyWidth,
        dragProxyHeight: (_g = dimensions === null || dimensions === void 0 ? void 0 : dimensions.dragProxyHeight) !== null && _g !== void 0 ? _g : ResolvedLayoutConfig.Dimensions.defaults.dragProxyHeight
      };
      return result;
    }
    Dimensions.resolve = resolve2;
  })(LayoutConfig2.Dimensions || (LayoutConfig2.Dimensions = {}));
  (function(Header2) {
    function resolve2(header, settings, labels) {
      var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k, _l, _m;
      let show;
      if ((header === null || header === void 0 ? void 0 : header.show) !== void 0) {
        show = header.show;
      } else {
        if (settings !== void 0 && settings.hasHeaders !== void 0) {
          show = settings.hasHeaders ? ResolvedLayoutConfig.Header.defaults.show : false;
        } else {
          show = ResolvedLayoutConfig.Header.defaults.show;
        }
      }
      const result = {
        show,
        popout: (_b = (_a = header === null || header === void 0 ? void 0 : header.popout) !== null && _a !== void 0 ? _a : labels === null || labels === void 0 ? void 0 : labels.popout) !== null && _b !== void 0 ? _b : (settings === null || settings === void 0 ? void 0 : settings.showPopoutIcon) === false ? false : ResolvedLayoutConfig.Header.defaults.popout,
        dock: (_d = (_c = header === null || header === void 0 ? void 0 : header.popin) !== null && _c !== void 0 ? _c : labels === null || labels === void 0 ? void 0 : labels.popin) !== null && _d !== void 0 ? _d : ResolvedLayoutConfig.Header.defaults.dock,
        maximise: (_f = (_e = header === null || header === void 0 ? void 0 : header.maximise) !== null && _e !== void 0 ? _e : labels === null || labels === void 0 ? void 0 : labels.maximise) !== null && _f !== void 0 ? _f : (settings === null || settings === void 0 ? void 0 : settings.showMaximiseIcon) === false ? false : ResolvedLayoutConfig.Header.defaults.maximise,
        close: (_h = (_g = header === null || header === void 0 ? void 0 : header.close) !== null && _g !== void 0 ? _g : labels === null || labels === void 0 ? void 0 : labels.close) !== null && _h !== void 0 ? _h : (settings === null || settings === void 0 ? void 0 : settings.showCloseIcon) === false ? false : ResolvedLayoutConfig.Header.defaults.close,
        minimise: (_k = (_j = header === null || header === void 0 ? void 0 : header.minimise) !== null && _j !== void 0 ? _j : labels === null || labels === void 0 ? void 0 : labels.minimise) !== null && _k !== void 0 ? _k : ResolvedLayoutConfig.Header.defaults.minimise,
        tabDropdown: (_m = (_l = header === null || header === void 0 ? void 0 : header.tabDropdown) !== null && _l !== void 0 ? _l : labels === null || labels === void 0 ? void 0 : labels.tabDropdown) !== null && _m !== void 0 ? _m : ResolvedLayoutConfig.Header.defaults.tabDropdown
      };
      return result;
    }
    Header2.resolve = resolve2;
  })(LayoutConfig2.Header || (LayoutConfig2.Header = {}));
  function isPopout(config) {
    return "parentId" in config || "indexInParent" in config || "window" in config;
  }
  LayoutConfig2.isPopout = isPopout;
  function resolve(layoutConfig) {
    if (isPopout(layoutConfig)) {
      return PopoutLayoutConfig.resolve(layoutConfig);
    } else {
      let root;
      if (layoutConfig.root !== void 0) {
        root = layoutConfig.root;
      } else {
        if (layoutConfig.content !== void 0 && layoutConfig.content.length > 0) {
          root = layoutConfig.content[0];
        } else {
          root = void 0;
        }
      }
      const config = {
        resolved: true,
        root: RootItemConfig.resolve(root),
        openPopouts: LayoutConfig2.resolveOpenPopouts(layoutConfig.openPopouts),
        dimensions: LayoutConfig2.Dimensions.resolve(layoutConfig.dimensions),
        settings: LayoutConfig2.Settings.resolve(layoutConfig.settings),
        header: LayoutConfig2.Header.resolve(layoutConfig.header, layoutConfig.settings, layoutConfig.labels)
      };
      return config;
    }
  }
  LayoutConfig2.resolve = resolve;
  function fromResolved(config) {
    const copiedConfig = ResolvedLayoutConfig.createCopy(config);
    const result = {
      root: copiedConfig.root,
      openPopouts: copiedConfig.openPopouts,
      dimensions: copiedConfig.dimensions,
      settings: copiedConfig.settings,
      header: copiedConfig.header
    };
    return result;
  }
  LayoutConfig2.fromResolved = fromResolved;
  function isResolved(configOrResolvedConfig) {
    const config = configOrResolvedConfig;
    return config.resolved !== void 0 && config.resolved === true;
  }
  LayoutConfig2.isResolved = isResolved;
  function resolveOpenPopouts(popoutConfigs) {
    if (popoutConfigs === void 0) {
      return [];
    } else {
      const count = popoutConfigs.length;
      const result = new Array(count);
      for (let i = 0; i < count; i++) {
        result[i] = PopoutLayoutConfig.resolve(popoutConfigs[i]);
      }
      return result;
    }
  }
  LayoutConfig2.resolveOpenPopouts = resolveOpenPopouts;
})(LayoutConfig || (LayoutConfig = {}));
var PopoutLayoutConfig;
(function(PopoutLayoutConfig2) {
  (function(Window) {
    function resolve2(window2, dimensions) {
      var _a, _b, _c, _d, _e, _f, _g, _h;
      let result;
      const defaults = ResolvedPopoutLayoutConfig.Window.defaults;
      if (window2 !== void 0) {
        result = {
          width: (_a = window2.width) !== null && _a !== void 0 ? _a : defaults.width,
          height: (_b = window2.height) !== null && _b !== void 0 ? _b : defaults.height,
          left: (_c = window2.left) !== null && _c !== void 0 ? _c : defaults.left,
          top: (_d = window2.top) !== null && _d !== void 0 ? _d : defaults.top
        };
      } else {
        result = {
          width: (_e = dimensions === null || dimensions === void 0 ? void 0 : dimensions.width) !== null && _e !== void 0 ? _e : defaults.width,
          height: (_f = dimensions === null || dimensions === void 0 ? void 0 : dimensions.height) !== null && _f !== void 0 ? _f : defaults.height,
          left: (_g = dimensions === null || dimensions === void 0 ? void 0 : dimensions.left) !== null && _g !== void 0 ? _g : defaults.left,
          top: (_h = dimensions === null || dimensions === void 0 ? void 0 : dimensions.top) !== null && _h !== void 0 ? _h : defaults.top
        };
      }
      return result;
    }
    Window.resolve = resolve2;
  })(PopoutLayoutConfig2.Window || (PopoutLayoutConfig2.Window = {}));
  function resolve(popoutConfig) {
    var _a, _b;
    let root;
    if (popoutConfig.root !== void 0) {
      root = popoutConfig.root;
    } else {
      if (popoutConfig.content !== void 0 && popoutConfig.content.length > 0) {
        root = popoutConfig.content[0];
      } else {
        root = void 0;
      }
    }
    const config = {
      root: RootItemConfig.resolve(root),
      openPopouts: LayoutConfig.resolveOpenPopouts(popoutConfig.openPopouts),
      settings: LayoutConfig.Settings.resolve(popoutConfig.settings),
      dimensions: LayoutConfig.Dimensions.resolve(popoutConfig.dimensions),
      header: LayoutConfig.Header.resolve(popoutConfig.header, popoutConfig.settings, popoutConfig.labels),
      parentId: (_a = popoutConfig.parentId) !== null && _a !== void 0 ? _a : null,
      indexInParent: (_b = popoutConfig.indexInParent) !== null && _b !== void 0 ? _b : null,
      window: PopoutLayoutConfig2.Window.resolve(popoutConfig.window, popoutConfig.dimensions),
      resolved: true
    };
    return config;
  }
  PopoutLayoutConfig2.resolve = resolve;
})(PopoutLayoutConfig || (PopoutLayoutConfig = {}));
class EventEmitter {
  constructor() {
    this._allEventSubscriptions = [];
    this._subscriptionsMap = new Map();
    this.unbind = this.removeEventListener;
    this.trigger = this.emit;
  }
  tryBubbleEvent(name, args) {
  }
  emit(eventName, ...args) {
    let subcriptions = this._subscriptionsMap.get(eventName);
    if (subcriptions !== void 0) {
      subcriptions = subcriptions.slice();
      for (let i = 0; i < subcriptions.length; i++) {
        const subscription = subcriptions[i];
        subscription(...args);
      }
    }
    this.emitAllEvent(eventName, args);
    this.tryBubbleEvent(eventName, args);
  }
  emitUnknown(eventName, ...args) {
    let subs = this._subscriptionsMap.get(eventName);
    if (subs !== void 0) {
      subs = subs.slice();
      for (let i = 0; i < subs.length; i++) {
        subs[i](...args);
      }
    }
    this.emitAllEvent(eventName, args);
    this.tryBubbleEvent(eventName, args);
  }
  emitBaseBubblingEvent(eventName) {
    const event = new EventEmitter.BubblingEvent(eventName, this);
    this.emitUnknown(eventName, event);
  }
  emitUnknownBubblingEvent(eventName) {
    const event = new EventEmitter.BubblingEvent(eventName, this);
    this.emitUnknown(eventName, event);
  }
  removeEventListener(eventName, callback) {
    const unknownCallback = callback;
    this.removeUnknownEventListener(eventName, unknownCallback);
  }
  off(eventName, callback) {
    this.removeEventListener(eventName, callback);
  }
  addEventListener(eventName, callback) {
    const unknownCallback = callback;
    this.addUnknownEventListener(eventName, unknownCallback);
  }
  on(eventName, callback) {
    this.addEventListener(eventName, callback);
  }
  addUnknownEventListener(eventName, callback) {
    if (eventName === EventEmitter.ALL_EVENT) {
      this._allEventSubscriptions.push(callback);
    } else {
      let subscriptions = this._subscriptionsMap.get(eventName);
      if (subscriptions !== void 0) {
        subscriptions.push(callback);
      } else {
        subscriptions = [callback];
        this._subscriptionsMap.set(eventName, subscriptions);
      }
    }
  }
  removeUnknownEventListener(eventName, callback) {
    if (eventName === EventEmitter.ALL_EVENT) {
      this.removeSubscription(eventName, this._allEventSubscriptions, callback);
    } else {
      const subscriptions = this._subscriptionsMap.get(eventName);
      if (subscriptions === void 0) {
        throw new Error("No subscribtions to unsubscribe for event " + eventName);
      } else {
        this.removeSubscription(eventName, subscriptions, callback);
      }
    }
  }
  removeSubscription(eventName, subscriptions, callback) {
    const idx = subscriptions.indexOf(callback);
    if (idx < 0) {
      throw new Error("Nothing to unbind for " + eventName);
    } else {
      subscriptions.splice(idx, 1);
    }
  }
  emitAllEvent(eventName, args) {
    const allEventSubscriptionsCount = this._allEventSubscriptions.length;
    if (allEventSubscriptionsCount > 0) {
      const unknownArgs = args.slice();
      unknownArgs.unshift(eventName);
      const allEventSubcriptions = this._allEventSubscriptions.slice();
      for (let i = 0; i < allEventSubscriptionsCount; i++) {
        allEventSubcriptions[i](...unknownArgs);
      }
    }
  }
}
(function(EventEmitter2) {
  EventEmitter2.ALL_EVENT = "__all";
  EventEmitter2.headerClickEventName = "stackHeaderClick";
  EventEmitter2.headerTouchStartEventName = "stackHeaderTouchStart";
  class BubblingEvent {
    constructor(_name, _target) {
      this._name = _name;
      this._target = _target;
      this._isPropagationStopped = false;
    }
    get name() {
      return this._name;
    }
    get target() {
      return this._target;
    }
    get origin() {
      return this._target;
    }
    get isPropagationStopped() {
      return this._isPropagationStopped;
    }
    stopPropagation() {
      this._isPropagationStopped = true;
    }
  }
  EventEmitter2.BubblingEvent = BubblingEvent;
  class ClickBubblingEvent extends BubblingEvent {
    constructor(name, target, _mouseEvent) {
      super(name, target);
      this._mouseEvent = _mouseEvent;
    }
    get mouseEvent() {
      return this._mouseEvent;
    }
  }
  EventEmitter2.ClickBubblingEvent = ClickBubblingEvent;
  class TouchStartBubblingEvent extends BubblingEvent {
    constructor(name, target, _touchEvent) {
      super(name, target);
      this._touchEvent = _touchEvent;
    }
    get touchEvent() {
      return this._touchEvent;
    }
  }
  EventEmitter2.TouchStartBubblingEvent = TouchStartBubblingEvent;
})(EventEmitter || (EventEmitter = {}));
var StyleConstants;
(function(StyleConstants2) {
  StyleConstants2.defaultComponentBaseZIndex = "auto";
  StyleConstants2.defaultComponentDragZIndex = "32";
  StyleConstants2.defaultComponentStackMaximisedZIndex = "41";
})(StyleConstants || (StyleConstants = {}));
class ComponentContainer extends EventEmitter {
  constructor(_config, _parent, _layoutManager, _element, _updateItemConfigEvent, _showEvent, _hideEvent, _focusEvent, _blurEvent) {
    super();
    this._config = _config;
    this._parent = _parent;
    this._layoutManager = _layoutManager;
    this._element = _element;
    this._updateItemConfigEvent = _updateItemConfigEvent;
    this._showEvent = _showEvent;
    this._hideEvent = _hideEvent;
    this._focusEvent = _focusEvent;
    this._blurEvent = _blurEvent;
    this._stackMaximised = false;
    this._width = 0;
    this._height = 0;
    this._visible = true;
    this._isShownWithZeroDimensions = true;
    this._componentType = _config.componentType;
    this._isClosable = _config.isClosable;
    this._initialState = _config.componentState;
    this._state = this._initialState;
    this._boundComponent = this.layoutManager.bindComponent(this, _config);
    this.updateElementPositionPropertyFromBoundComponent();
  }
  get width() {
    return this._width;
  }
  get height() {
    return this._height;
  }
  get parent() {
    return this._parent;
  }
  get componentName() {
    return this._componentType;
  }
  get componentType() {
    return this._componentType;
  }
  get virtual() {
    return this._boundComponent.virtual;
  }
  get component() {
    return this._boundComponent.component;
  }
  get tab() {
    return this._tab;
  }
  get title() {
    return this._parent.title;
  }
  get layoutManager() {
    return this._layoutManager;
  }
  get isHidden() {
    return !this._visible;
  }
  get visible() {
    return this._visible;
  }
  get state() {
    return this._state;
  }
  get initialState() {
    return this._initialState;
  }
  get element() {
    return this._element;
  }
  destroy() {
    this.releaseComponent();
    this.stateRequestEvent = void 0;
    this.emit("destroy");
  }
  getElement() {
    return this._element;
  }
  hide() {
    this._hideEvent();
  }
  show() {
    this._showEvent();
  }
  focus(suppressEvent = false) {
    this._focusEvent(suppressEvent);
  }
  blur(suppressEvent = false) {
    this._blurEvent(suppressEvent);
  }
  setSize(width, height) {
    let ancestorItem = this._parent;
    if (ancestorItem.isColumn || ancestorItem.isRow || ancestorItem.parent === null) {
      throw new AssertError("ICSSPRC", "ComponentContainer cannot have RowColumn Parent");
    } else {
      let ancestorChildItem;
      do {
        ancestorChildItem = ancestorItem;
        ancestorItem = ancestorItem.parent;
      } while (ancestorItem !== null && !ancestorItem.isColumn && !ancestorItem.isRow);
      if (ancestorItem === null) {
        return false;
      } else {
        const direction = ancestorItem.isColumn ? "height" : "width";
        const currentSize = this[direction];
        if (currentSize === null) {
          throw new UnexpectedNullError("ICSSCS11194");
        } else {
          const newSize = direction === "height" ? height : width;
          const totalPixel = currentSize * (1 / (ancestorChildItem[direction] / 100));
          const percentage = newSize / totalPixel * 100;
          const delta = (ancestorChildItem[direction] - percentage) / (ancestorItem.contentItems.length - 1);
          for (let i = 0; i < ancestorItem.contentItems.length; i++) {
            if (ancestorItem.contentItems[i] === ancestorChildItem) {
              ancestorItem.contentItems[i][direction] = percentage;
            } else {
              ancestorItem.contentItems[i][direction] += delta;
            }
          }
          ancestorItem.updateSize();
          return true;
        }
      }
    }
  }
  close() {
    if (this._isClosable) {
      this.emit("close");
      this._parent.close();
    }
  }
  replaceComponent(itemConfig) {
    this.releaseComponent();
    if (!ItemConfig.isComponent(itemConfig)) {
      throw new Error("ReplaceComponent not passed a component ItemConfig");
    } else {
      const config = ComponentItemConfig.resolve(itemConfig);
      this._initialState = config.componentState;
      this._state = this._initialState;
      this._componentType = config.componentType;
      this._updateItemConfigEvent(config);
      this._boundComponent = this.layoutManager.bindComponent(this, config);
      this.updateElementPositionPropertyFromBoundComponent();
      if (this._boundComponent.virtual) {
        if (this.virtualVisibilityChangeRequiredEvent !== void 0) {
          this.virtualVisibilityChangeRequiredEvent(this, this._visible);
        }
        if (this.virtualRectingRequiredEvent !== void 0) {
          this._layoutManager.fireBeforeVirtualRectingEvent(1);
          try {
            this.virtualRectingRequiredEvent(this, this._width, this._height);
          } finally {
            this._layoutManager.fireAfterVirtualRectingEvent();
          }
        }
        if (this.virtualZIndexChangeRequiredEvent !== void 0) {
          this.virtualZIndexChangeRequiredEvent(this, LogicalZIndex.base, StyleConstants.defaultComponentBaseZIndex);
        }
      }
      this.emit("stateChanged");
    }
  }
  getState() {
    return this._state;
  }
  extendState(state) {
    const extendedState = deepExtend(this._state, state);
    this.setState(extendedState);
  }
  setState(state) {
    this._state = state;
    this._parent.emitBaseBubblingEvent("stateChanged");
  }
  setTitle(title) {
    this._parent.setTitle(title);
  }
  setTab(tab) {
    this._tab = tab;
    this.emit("tab", tab);
  }
  setVisibility(value) {
    if (this._boundComponent.virtual) {
      if (this.virtualVisibilityChangeRequiredEvent !== void 0) {
        this.virtualVisibilityChangeRequiredEvent(this, value);
      }
    }
    if (value) {
      if (!this._visible) {
        this._visible = true;
        if (this._height === 0 && this._width === 0) {
          this._isShownWithZeroDimensions = true;
        } else {
          this._isShownWithZeroDimensions = false;
          this.setSizeToNodeSize(this._width, this._height, true);
          this.emitShow();
        }
      } else {
        if (this._isShownWithZeroDimensions && (this._height !== 0 || this._width !== 0)) {
          this._isShownWithZeroDimensions = false;
          this.setSizeToNodeSize(this._width, this._height, true);
          this.emitShow();
        }
      }
    } else {
      if (this._visible) {
        this._visible = false;
        this._isShownWithZeroDimensions = false;
        this.emitHide();
      }
    }
  }
  enterDragMode(width, height) {
    this._width = width;
    this._height = height;
    setElementWidth(this._element, width);
    setElementHeight(this._element, height);
    if (this.virtualZIndexChangeRequiredEvent !== void 0) {
      this.virtualZIndexChangeRequiredEvent(this, LogicalZIndex.drag, StyleConstants.defaultComponentDragZIndex);
    }
    this.drag();
  }
  exitDragMode() {
    if (this.virtualZIndexChangeRequiredEvent !== void 0) {
      this.virtualZIndexChangeRequiredEvent(this, LogicalZIndex.base, StyleConstants.defaultComponentBaseZIndex);
    }
  }
  enterStackMaximised() {
    this._stackMaximised = true;
    if (this.virtualZIndexChangeRequiredEvent !== void 0) {
      this.virtualZIndexChangeRequiredEvent(this, LogicalZIndex.stackMaximised, StyleConstants.defaultComponentStackMaximisedZIndex);
    }
  }
  exitStackMaximised() {
    if (this.virtualZIndexChangeRequiredEvent !== void 0) {
      this.virtualZIndexChangeRequiredEvent(this, LogicalZIndex.base, StyleConstants.defaultComponentBaseZIndex);
    }
    this._stackMaximised = false;
  }
  drag() {
    if (this._boundComponent.virtual) {
      if (this.virtualRectingRequiredEvent !== void 0) {
        this._layoutManager.fireBeforeVirtualRectingEvent(1);
        try {
          this.virtualRectingRequiredEvent(this, this._width, this._height);
        } finally {
          this._layoutManager.fireAfterVirtualRectingEvent();
        }
      }
    }
  }
  setSizeToNodeSize(width, height, force) {
    if (width !== this._width || height !== this._height || force) {
      this._width = width;
      this._height = height;
      setElementWidth(this._element, width);
      setElementHeight(this._element, height);
      if (this._boundComponent.virtual) {
        this.addVirtualSizedContainerToLayoutManager();
      } else {
        this.emit("resize");
        this.checkShownFromZeroDimensions();
      }
    }
  }
  notifyVirtualRectingRequired() {
    if (this.virtualRectingRequiredEvent !== void 0) {
      this.virtualRectingRequiredEvent(this, this._width, this._height);
      this.emit("resize");
      this.checkShownFromZeroDimensions();
    }
  }
  updateElementPositionPropertyFromBoundComponent() {
    if (this._boundComponent.virtual) {
      this._element.style.position = "static";
    } else {
      this._element.style.position = "";
    }
  }
  addVirtualSizedContainerToLayoutManager() {
    this._layoutManager.beginVirtualSizedContainerAdding();
    try {
      this._layoutManager.addVirtualSizedContainer(this);
    } finally {
      this._layoutManager.endVirtualSizedContainerAdding();
    }
  }
  checkShownFromZeroDimensions() {
    if (this._isShownWithZeroDimensions && (this._height !== 0 || this._width !== 0)) {
      this._isShownWithZeroDimensions = false;
      this.emitShow();
    }
  }
  emitShow() {
    this.emit("shown");
    this.emit("show");
  }
  emitHide() {
    this.emit("hide");
  }
  releaseComponent() {
    if (this._stackMaximised) {
      this.exitStackMaximised();
    }
    this.emit("beforeComponentRelease", this._boundComponent.component);
    this.layoutManager.unbindComponent(this, this._boundComponent.virtual, this._boundComponent.component);
  }
}
class BrowserPopout extends EventEmitter {
  constructor(_config, _initialWindowSize, _layoutManager) {
    super();
    this._config = _config;
    this._initialWindowSize = _initialWindowSize;
    this._layoutManager = _layoutManager;
    this._isInitialised = false;
    this._popoutWindow = null;
    this.createWindow();
  }
  toConfig() {
    var _a, _b;
    if (this._isInitialised === false) {
      throw new Error("Can't create config, layout not yet initialised");
    }
    const glInstance = this.getGlInstance();
    const glInstanceConfig = glInstance.saveLayout();
    let left;
    let top;
    if (this._popoutWindow === null) {
      left = null;
      top = null;
    } else {
      left = (_a = this._popoutWindow.screenX) !== null && _a !== void 0 ? _a : this._popoutWindow.screenLeft;
      top = (_b = this._popoutWindow.screenY) !== null && _b !== void 0 ? _b : this._popoutWindow.screenTop;
    }
    const window2 = {
      width: this.getGlInstance().width,
      height: this.getGlInstance().height,
      left,
      top
    };
    const config = {
      root: glInstanceConfig.root,
      openPopouts: glInstanceConfig.openPopouts,
      settings: glInstanceConfig.settings,
      dimensions: glInstanceConfig.dimensions,
      header: glInstanceConfig.header,
      window: window2,
      parentId: this._config.parentId,
      indexInParent: this._config.indexInParent,
      resolved: true
    };
    return config;
  }
  getGlInstance() {
    if (this._popoutWindow === null) {
      throw new UnexpectedNullError("BPGGI24693");
    }
    return this._popoutWindow.__glInstance;
  }
  getWindow() {
    if (this._popoutWindow === null) {
      throw new UnexpectedNullError("BPGW087215");
    }
    return this._popoutWindow;
  }
  close() {
    if (this.getGlInstance()) {
      this.getGlInstance().closeWindow();
    } else {
      try {
        this.getWindow().close();
      } catch (e) {
      }
    }
  }
  popIn() {
    let parentItem;
    let index = this._config.indexInParent;
    if (!this._config.parentId) {
      return;
    }
    const glInstanceLayoutConfig = this.getGlInstance().saveLayout();
    const copiedGlInstanceLayoutConfig = deepExtend({}, glInstanceLayoutConfig);
    const copiedRoot = copiedGlInstanceLayoutConfig.root;
    if (copiedRoot === void 0) {
      throw new UnexpectedUndefinedError("BPPIR19998");
    }
    const groundItem = this._layoutManager.groundItem;
    if (groundItem === void 0) {
      throw new UnexpectedUndefinedError("BPPIG34972");
    }
    parentItem = groundItem.getItemsByPopInParentId(this._config.parentId)[0];
    if (!parentItem) {
      if (groundItem.contentItems.length > 0) {
        parentItem = groundItem.contentItems[0];
      } else {
        parentItem = groundItem;
      }
      index = 0;
    }
    const newContentItem = this._layoutManager.createAndInitContentItem(copiedRoot, parentItem);
    parentItem.addChild(newContentItem, index);
    if (this._layoutManager.layoutConfig.settings.popInOnClose) {
      this._onClose();
    } else {
      this.close();
    }
  }
  createWindow() {
    const url = this.createUrl();
    const target = Math.floor(Math.random() * 1e6).toString(36);
    const features = this.serializeWindowFeatures({
      width: this._initialWindowSize.width,
      height: this._initialWindowSize.height,
      innerWidth: this._initialWindowSize.width,
      innerHeight: this._initialWindowSize.height,
      menubar: "no",
      toolbar: "no",
      location: "no",
      personalbar: "no",
      resizable: "yes",
      scrollbars: "no",
      status: "no"
    });
    this._popoutWindow = globalThis.open(url, target, features);
    if (!this._popoutWindow) {
      if (this._layoutManager.layoutConfig.settings.blockedPopoutsThrowError === true) {
        const error = new PopoutBlockedError("Popout blocked");
        throw error;
      } else {
        return;
      }
    }
    this._popoutWindow.addEventListener("load", () => this.positionWindow(), {passive: true});
    this._popoutWindow.addEventListener("beforeunload", () => {
      if (this._layoutManager.layoutConfig.settings.popInOnClose) {
        this.popIn();
      } else {
        this._onClose();
      }
    }, {passive: true});
    this._checkReadyInterval = setInterval(() => this.checkReady(), 10);
  }
  checkReady() {
    if (this._popoutWindow === null) {
      throw new UnexpectedNullError("BPCR01844");
    } else {
      if (this._popoutWindow.__glInstance && this._popoutWindow.__glInstance.isInitialised) {
        this.onInitialised();
        if (this._checkReadyInterval !== void 0) {
          clearInterval(this._checkReadyInterval);
          this._checkReadyInterval = void 0;
        }
      }
    }
  }
  serializeWindowFeatures(windowOptions) {
    const windowOptionsString = [];
    for (const key in windowOptions) {
      windowOptionsString.push(key + "=" + windowOptions[key].toString());
    }
    return windowOptionsString.join(",");
  }
  createUrl() {
    const storageKey = "gl-window-config-" + getUniqueId();
    const config = ResolvedLayoutConfig.minifyConfig(this._config);
    try {
      localStorage.setItem(storageKey, JSON.stringify(config));
    } catch (e) {
      throw new Error("Error while writing to localStorage " + e.toString());
    }
    const url = new URL(location.href);
    url.searchParams.set("gl-window", storageKey);
    return url.toString();
  }
  positionWindow() {
    if (this._popoutWindow === null) {
      throw new Error("BrowserPopout.positionWindow: null popoutWindow");
    } else {
      this._popoutWindow.moveTo(this._initialWindowSize.left, this._initialWindowSize.top);
      this._popoutWindow.focus();
    }
  }
  onInitialised() {
    this._isInitialised = true;
    this.getGlInstance().on("popIn", () => this.popIn());
    this.emit("initialised");
  }
  _onClose() {
    setTimeout(() => this.emit("closed"), 50);
  }
}
function getJQueryOffset(element) {
  const rect = element.getBoundingClientRect();
  return {
    top: rect.top + document.body.scrollTop,
    left: rect.left + document.body.scrollLeft
  };
}
class ContentItem extends EventEmitter {
  constructor(layoutManager, config, _parent, _element) {
    super();
    this.layoutManager = layoutManager;
    this._parent = _parent;
    this._element = _element;
    this._popInParentIds = [];
    this._type = config.type;
    this._id = config.id;
    this._isInitialised = false;
    this.isGround = false;
    this.isRow = false;
    this.isColumn = false;
    this.isStack = false;
    this.isComponent = false;
    this.width = config.width;
    this.minWidth = config.minWidth;
    this.height = config.height;
    this.minHeight = config.minHeight;
    this._isClosable = config.isClosable;
    this._pendingEventPropagations = {};
    this._throttledEvents = ["stateChanged"];
    this._contentItems = this.createContentItems(config.content);
  }
  get type() {
    return this._type;
  }
  get id() {
    return this._id;
  }
  set id(value) {
    this._id = value;
  }
  get popInParentIds() {
    return this._popInParentIds;
  }
  get parent() {
    return this._parent;
  }
  get contentItems() {
    return this._contentItems;
  }
  get isClosable() {
    return this._isClosable;
  }
  get element() {
    return this._element;
  }
  get isInitialised() {
    return this._isInitialised;
  }
  static isStack(item) {
    return item.isStack;
  }
  static isComponentItem(item) {
    return item.isComponent;
  }
  static isComponentParentableItem(item) {
    return item.isStack || item.isGround;
  }
  removeChild(contentItem, keepChild = false) {
    const index = this._contentItems.indexOf(contentItem);
    if (index === -1) {
      throw new Error("Can't remove child item. Unknown content item");
    }
    if (!keepChild) {
      this._contentItems[index].destroy();
    }
    this._contentItems.splice(index, 1);
    if (this._contentItems.length > 0) {
      this.updateSize();
    } else {
      if (!this.isGround && this._isClosable === true) {
        if (this._parent === null) {
          throw new UnexpectedNullError("CIUC00874");
        } else {
          this._parent.removeChild(this);
        }
      }
    }
  }
  addChild(contentItem, index, suspendResize) {
    index !== null && index !== void 0 ? index : index = this._contentItems.length;
    this._contentItems.splice(index, 0, contentItem);
    contentItem.setParent(this);
    if (this._isInitialised === true && contentItem._isInitialised === false) {
      contentItem.init();
    }
    return index;
  }
  replaceChild(oldChild, newChild, destroyOldChild = false) {
    const index = this._contentItems.indexOf(oldChild);
    const parentNode = oldChild._element.parentNode;
    if (index === -1) {
      throw new AssertError("CIRCI23232", "Can't replace child. oldChild is not child of this");
    }
    if (parentNode === null) {
      throw new UnexpectedNullError("CIRCP23232");
    } else {
      parentNode.replaceChild(newChild._element, oldChild._element);
      if (destroyOldChild === true) {
        oldChild._parent = null;
        oldChild.destroy();
      }
      this._contentItems[index] = newChild;
      newChild.setParent(this);
      newChild.height = oldChild.height;
      newChild.width = oldChild.width;
      if (newChild._parent === null) {
        throw new UnexpectedNullError("CIRCNC45699");
      } else {
        if (newChild._parent._isInitialised === true && newChild._isInitialised === false) {
          newChild.init();
        }
        this.updateSize();
      }
    }
  }
  remove() {
    if (this._parent === null) {
      throw new UnexpectedNullError("CIR11110");
    } else {
      this._parent.removeChild(this);
    }
  }
  popout() {
    const parentId = getUniqueId();
    const browserPopout = this.layoutManager.createPopoutFromContentItem(this, void 0, parentId, void 0);
    this.emitBaseBubblingEvent("stateChanged");
    return browserPopout;
  }
  calculateConfigContent() {
    const contentItems = this._contentItems;
    const count = contentItems.length;
    const result = new Array(count);
    for (let i = 0; i < count; i++) {
      const item = contentItems[i];
      result[i] = item.toConfig();
    }
    return result;
  }
  highlightDropZone(x, y, area) {
    const dropTargetIndicator = this.layoutManager.dropTargetIndicator;
    if (dropTargetIndicator === null) {
      throw new UnexpectedNullError("ACIHDZ5593");
    } else {
      dropTargetIndicator.highlightArea(area);
    }
  }
  onDrop(contentItem, area) {
    this.addChild(contentItem);
  }
  show() {
    setElementDisplayVisibility(this._element, true);
    this.layoutManager.updateSizeFromContainer();
    for (let i = 0; i < this._contentItems.length; i++) {
      this._contentItems[i].show();
    }
  }
  destroy() {
    for (let i = 0; i < this._contentItems.length; i++) {
      this._contentItems[i].destroy();
    }
    this._contentItems = [];
    this.emitBaseBubblingEvent("beforeItemDestroyed");
    this._element.remove();
    this.emitBaseBubblingEvent("itemDestroyed");
  }
  getElementArea(element) {
    element = element !== null && element !== void 0 ? element : this._element;
    const offset = getJQueryOffset(element);
    const width = element.offsetWidth;
    const height = element.offsetHeight;
    return {
      x1: offset.left + 1,
      y1: offset.top + 1,
      x2: offset.left + width - 1,
      y2: offset.top + height - 1,
      surface: width * height,
      contentItem: this
    };
  }
  init() {
    this._isInitialised = true;
    this.emitBaseBubblingEvent("itemCreated");
    this.emitUnknownBubblingEvent(this.type + "Created");
  }
  setParent(parent) {
    this._parent = parent;
  }
  addPopInParentId(id) {
    if (!this.popInParentIds.includes(id)) {
      this.popInParentIds.push(id);
    }
  }
  initContentItems() {
    for (let i = 0; i < this._contentItems.length; i++) {
      this._contentItems[i].init();
    }
  }
  hide() {
    setElementDisplayVisibility(this._element, false);
    this.layoutManager.updateSizeFromContainer();
  }
  updateContentItemsSize() {
    for (let i = 0; i < this._contentItems.length; i++) {
      this._contentItems[i].updateSize();
    }
  }
  createContentItems(content) {
    const count = content.length;
    const result = new Array(count);
    for (let i = 0; i < content.length; i++) {
      result[i] = this.layoutManager.createContentItem(content[i], this);
    }
    return result;
  }
  propagateEvent(name, args) {
    if (args.length === 1) {
      const event = args[0];
      if (event instanceof EventEmitter.BubblingEvent && event.isPropagationStopped === false && this._isInitialised === true) {
        if (this.isGround === false && this._parent) {
          this._parent.emitUnknown(name, event);
        } else {
          this.scheduleEventPropagationToLayoutManager(name, event);
        }
      }
    }
  }
  tryBubbleEvent(name, args) {
    if (args.length === 1) {
      const event = args[0];
      if (event instanceof EventEmitter.BubblingEvent && event.isPropagationStopped === false && this._isInitialised === true) {
        if (this.isGround === false && this._parent) {
          this._parent.emitUnknown(name, event);
        } else {
          this.scheduleEventPropagationToLayoutManager(name, event);
        }
      }
    }
  }
  scheduleEventPropagationToLayoutManager(name, event) {
    if (this._throttledEvents.indexOf(name) === -1) {
      this.layoutManager.emitUnknown(name, event);
    } else {
      if (this._pendingEventPropagations[name] !== true) {
        this._pendingEventPropagations[name] = true;
        globalThis.requestAnimationFrame(() => this.propagateEventToLayoutManager(name, event));
      }
    }
  }
  propagateEventToLayoutManager(name, event) {
    this._pendingEventPropagations[name] = false;
    this.layoutManager.emitUnknown(name, event);
  }
}
class ComponentItem extends ContentItem {
  constructor(layoutManager, config, _parentItem) {
    super(layoutManager, config, _parentItem, document.createElement("div"));
    this._parentItem = _parentItem;
    this._focused = false;
    this.isComponent = true;
    this._reorderEnabled = config.reorderEnabled;
    this.applyUpdatableConfig(config);
    this._initialWantMaximise = config.maximised;
    const containerElement = document.createElement("div");
    containerElement.classList.add("lm_content");
    this.element.appendChild(containerElement);
    this._container = new ComponentContainer(config, this, layoutManager, containerElement, (itemConfig) => this.handleUpdateItemConfigEvent(itemConfig), () => this.show(), () => this.hide(), (suppressEvent) => this.focus(suppressEvent), (suppressEvent) => this.blur(suppressEvent));
  }
  get componentName() {
    return this._container.componentType;
  }
  get componentType() {
    return this._container.componentType;
  }
  get reorderEnabled() {
    return this._reorderEnabled;
  }
  get initialWantMaximise() {
    return this._initialWantMaximise;
  }
  get component() {
    return this._container.component;
  }
  get container() {
    return this._container;
  }
  get parentItem() {
    return this._parentItem;
  }
  get headerConfig() {
    return this._headerConfig;
  }
  get title() {
    return this._title;
  }
  get tab() {
    return this._tab;
  }
  get focused() {
    return this._focused;
  }
  destroy() {
    this._container.destroy();
    super.destroy();
  }
  applyUpdatableConfig(config) {
    this.setTitle(config.title);
    this._headerConfig = config.header;
  }
  toConfig() {
    const stateRequestEvent = this._container.stateRequestEvent;
    const state = stateRequestEvent === void 0 ? this._container.state : stateRequestEvent();
    const result = {
      type: ItemType.component,
      content: [],
      width: this.width,
      minWidth: this.minWidth,
      height: this.height,
      minHeight: this.minHeight,
      id: this.id,
      maximised: false,
      isClosable: this.isClosable,
      reorderEnabled: this._reorderEnabled,
      title: this._title,
      header: ResolvedHeaderedItemConfig.Header.createCopy(this._headerConfig),
      componentType: ResolvedComponentItemConfig.copyComponentType(this.componentType),
      componentState: state
    };
    return result;
  }
  close() {
    if (this.parent === null) {
      throw new UnexpectedNullError("CIC68883");
    } else {
      this.parent.removeChild(this, false);
    }
  }
  enterDragMode(width, height) {
    setElementWidth(this.element, width);
    setElementHeight(this.element, height);
    this._container.enterDragMode(width, height);
  }
  exitDragMode() {
    this._container.exitDragMode();
  }
  enterStackMaximised() {
    this._container.enterStackMaximised();
  }
  exitStackMaximised() {
    this._container.exitStackMaximised();
  }
  drag() {
    this._container.drag();
  }
  updateSize() {
    this.updateNodeSize();
  }
  init() {
    this.updateNodeSize();
    super.init();
    this._container.emit("open");
    this.initContentItems();
  }
  setTitle(title) {
    this._title = title;
    this.emit("titleChanged", title);
    this.emit("stateChanged");
  }
  setTab(tab) {
    this._tab = tab;
    this.emit("tab", tab);
    this._container.setTab(tab);
  }
  hide() {
    super.hide();
    this._container.setVisibility(false);
  }
  show() {
    super.show();
    this._container.setVisibility(true);
  }
  focus(suppressEvent = false) {
    this.parentItem.setActiveComponentItem(this, true, suppressEvent);
  }
  setFocused(suppressEvent) {
    this._focused = true;
    this.tab.setFocused();
    if (!suppressEvent) {
      this.emitBaseBubblingEvent("focus");
    }
  }
  blur(suppressEvent = false) {
    if (this._focused) {
      this.layoutManager.setFocusedComponentItem(void 0, suppressEvent);
    }
  }
  setBlurred(suppressEvent) {
    this._focused = false;
    this.tab.setBlurred();
    if (!suppressEvent) {
      this.emitBaseBubblingEvent("blur");
    }
  }
  setParent(parent) {
    this._parentItem = parent;
    super.setParent(parent);
  }
  handleUpdateItemConfigEvent(itemConfig) {
    this.applyUpdatableConfig(itemConfig);
  }
  updateNodeSize() {
    if (this.element.style.display !== "none") {
      const {width, height} = getElementWidthAndHeight(this.element);
      this._container.setSizeToNodeSize(width, height, false);
    }
  }
}
class ComponentParentableItem extends ContentItem {
  constructor() {
    super(...arguments);
    this._focused = false;
  }
  get focused() {
    return this._focused;
  }
  setFocusedValue(value) {
    this._focused = value;
  }
}
class DragListener extends EventEmitter {
  constructor(_eElement, extraAllowableChildTargets) {
    super();
    this._eElement = _eElement;
    this._pointerTracking = false;
    this._pointerDownEventListener = (ev) => this.onPointerDown(ev);
    this._pointerMoveEventListener = (ev) => this.onPointerMove(ev);
    this._pointerUpEventListener = (ev) => this.onPointerUp(ev);
    this._timeout = void 0;
    this._allowableTargets = [_eElement, ...extraAllowableChildTargets];
    this._oDocument = document;
    this._eBody = document.body;
    this._nDelay = 1800;
    this._nDistance = 10;
    this._nX = 0;
    this._nY = 0;
    this._nOriginalX = 0;
    this._nOriginalY = 0;
    this._dragging = false;
    this._eElement.addEventListener("pointerdown", this._pointerDownEventListener, {passive: true});
  }
  destroy() {
    this.checkRemovePointerTrackingEventListeners();
    this._eElement.removeEventListener("pointerdown", this._pointerDownEventListener);
  }
  cancelDrag() {
    this.processDragStop(void 0);
  }
  onPointerDown(oEvent) {
    if (this._allowableTargets.includes(oEvent.target) && oEvent.isPrimary) {
      const coordinates = this.getPointerCoordinates(oEvent);
      this.processPointerDown(coordinates);
    }
  }
  processPointerDown(coordinates) {
    this._nOriginalX = coordinates.x;
    this._nOriginalY = coordinates.y;
    this._oDocument.addEventListener("pointermove", this._pointerMoveEventListener);
    this._oDocument.addEventListener("pointerup", this._pointerUpEventListener, {passive: true});
    this._pointerTracking = true;
    this._timeout = setTimeout(() => {
      try {
        this.startDrag();
      } catch (err) {
        console.error(err);
        throw err;
      }
    }, this._nDelay);
  }
  onPointerMove(oEvent) {
    if (this._pointerTracking) {
      this.processDragMove(oEvent);
      oEvent.preventDefault();
    }
  }
  processDragMove(dragEvent) {
    this._nX = dragEvent.pageX - this._nOriginalX;
    this._nY = dragEvent.pageY - this._nOriginalY;
    if (this._dragging === false) {
      if (Math.abs(this._nX) > this._nDistance || Math.abs(this._nY) > this._nDistance) {
        this.startDrag();
      }
    }
    if (this._dragging) {
      this.emit("drag", this._nX, this._nY, dragEvent);
    }
  }
  onPointerUp(oEvent) {
    this.processDragStop(oEvent);
  }
  processDragStop(dragEvent) {
    var _a;
    if (this._timeout !== void 0) {
      clearTimeout(this._timeout);
      this._timeout = void 0;
    }
    this.checkRemovePointerTrackingEventListeners();
    if (this._dragging === true) {
      this._eBody.classList.remove("lm_dragging");
      this._eElement.classList.remove("lm_dragging");
      (_a = this._oDocument.querySelector("iframe")) === null || _a === void 0 ? void 0 : _a.style.setProperty("pointer-events", "");
      this._dragging = false;
      this.emit("dragStop", dragEvent);
    }
  }
  checkRemovePointerTrackingEventListeners() {
    if (this._pointerTracking) {
      this._oDocument.removeEventListener("pointermove", this._pointerMoveEventListener);
      this._oDocument.removeEventListener("pointerup", this._pointerUpEventListener);
      this._pointerTracking = false;
    }
  }
  startDrag() {
    var _a;
    if (this._timeout !== void 0) {
      clearTimeout(this._timeout);
      this._timeout = void 0;
    }
    this._dragging = true;
    this._eBody.classList.add("lm_dragging");
    this._eElement.classList.add("lm_dragging");
    (_a = this._oDocument.querySelector("iframe")) === null || _a === void 0 ? void 0 : _a.style.setProperty("pointer-events", "none");
    this.emit("dragStart", this._nOriginalX, this._nOriginalY);
  }
  getPointerCoordinates(event) {
    const result = {
      x: event.pageX,
      y: event.pageY
    };
    return result;
  }
}
class Splitter {
  constructor(_isVertical, _size, grabSize) {
    this._isVertical = _isVertical;
    this._size = _size;
    this._grabSize = grabSize < this._size ? this._size : grabSize;
    this._element = document.createElement("div");
    this._element.classList.add("lm_splitter");
    const dragHandleElement = document.createElement("div");
    dragHandleElement.classList.add("lm_drag_handle");
    const handleExcessSize = this._grabSize - this._size;
    const handleExcessPos = handleExcessSize / 2;
    if (this._isVertical) {
      dragHandleElement.style.top = numberToPixels(-handleExcessPos);
      dragHandleElement.style.height = numberToPixels(this._size + handleExcessSize);
      this._element.classList.add("lm_vertical");
      this._element.style.height = numberToPixels(this._size);
    } else {
      dragHandleElement.style.left = numberToPixels(-handleExcessPos);
      dragHandleElement.style.width = numberToPixels(this._size + handleExcessSize);
      this._element.classList.add("lm_horizontal");
      this._element.style.width = numberToPixels(this._size);
    }
    this._element.appendChild(dragHandleElement);
    this._dragListener = new DragListener(this._element, [dragHandleElement]);
  }
  get element() {
    return this._element;
  }
  destroy() {
    this._element.remove();
  }
  on(eventName, callback) {
    this._dragListener.on(eventName, callback);
  }
}
class RowOrColumn extends ContentItem {
  constructor(isColumn, layoutManager, config, _rowOrColumnParent) {
    super(layoutManager, config, _rowOrColumnParent, RowOrColumn.createElement(document, isColumn));
    this._rowOrColumnParent = _rowOrColumnParent;
    this._splitter = [];
    this.isRow = !isColumn;
    this.isColumn = isColumn;
    this._childElementContainer = this.element;
    this._splitterSize = layoutManager.layoutConfig.dimensions.borderWidth;
    this._splitterGrabSize = layoutManager.layoutConfig.dimensions.borderGrabWidth;
    this._isColumn = isColumn;
    this._dimension = isColumn ? "height" : "width";
    this._splitterPosition = null;
    this._splitterMinPosition = null;
    this._splitterMaxPosition = null;
    switch (config.type) {
      case ItemType.row:
      case ItemType.column:
        this._configType = config.type;
        break;
      default:
        throw new AssertError("ROCCCT00925");
    }
  }
  newComponent(componentType, componentState, title, index) {
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    return this.newItem(itemConfig, index);
  }
  addComponent(componentType, componentState, title, index) {
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    return this.addItem(itemConfig, index);
  }
  newItem(itemConfig, index) {
    index = this.addItem(itemConfig, index);
    const createdItem = this.contentItems[index];
    if (ContentItem.isStack(createdItem) && ItemConfig.isComponent(itemConfig)) {
      return createdItem.contentItems[0];
    } else {
      return createdItem;
    }
  }
  addItem(itemConfig, index) {
    this.layoutManager.checkMinimiseMaximisedStack();
    const resolvedItemConfig = ItemConfig.resolve(itemConfig);
    const contentItem = this.layoutManager.createAndInitContentItem(resolvedItemConfig, this);
    return this.addChild(contentItem, index, false);
  }
  addChild(contentItem, index, suspendResize) {
    if (index === void 0) {
      index = this.contentItems.length;
    }
    if (this.contentItems.length > 0) {
      const splitterElement = this.createSplitter(Math.max(0, index - 1)).element;
      if (index > 0) {
        this.contentItems[index - 1].element.insertAdjacentElement("afterend", splitterElement);
        splitterElement.insertAdjacentElement("afterend", contentItem.element);
      } else {
        this.contentItems[0].element.insertAdjacentElement("beforebegin", splitterElement);
        splitterElement.insertAdjacentElement("beforebegin", contentItem.element);
      }
    } else {
      this._childElementContainer.appendChild(contentItem.element);
    }
    super.addChild(contentItem, index);
    const newItemSize = 1 / this.contentItems.length * 100;
    if (suspendResize === true) {
      this.emitBaseBubblingEvent("stateChanged");
      return index;
    }
    for (let i = 0; i < this.contentItems.length; i++) {
      if (this.contentItems[i] === contentItem) {
        contentItem[this._dimension] = newItemSize;
      } else {
        const itemSize = this.contentItems[i][this._dimension] *= (100 - newItemSize) / 100;
        this.contentItems[i][this._dimension] = itemSize;
      }
    }
    this.updateSize();
    this.emitBaseBubblingEvent("stateChanged");
    return index;
  }
  removeChild(contentItem, keepChild) {
    const index = this.contentItems.indexOf(contentItem);
    const splitterIndex = Math.max(index - 1, 0);
    if (index === -1) {
      throw new Error("Can't remove child. ContentItem is not child of this Row or Column");
    }
    if (this._splitter[splitterIndex]) {
      this._splitter[splitterIndex].destroy();
      this._splitter.splice(splitterIndex, 1);
    }
    super.removeChild(contentItem, keepChild);
    if (this.contentItems.length === 1 && this.isClosable === true) {
      const childItem = this.contentItems[0];
      this.contentItems.length = 0;
      this._rowOrColumnParent.replaceChild(this, childItem, true);
    } else {
      this.updateSize();
      this.emitBaseBubblingEvent("stateChanged");
    }
  }
  replaceChild(oldChild, newChild) {
    const size = oldChild[this._dimension];
    super.replaceChild(oldChild, newChild);
    newChild[this._dimension] = size;
    this.updateSize();
    this.emitBaseBubblingEvent("stateChanged");
  }
  updateSize() {
    this.layoutManager.beginVirtualSizedContainerAdding();
    try {
      this.updateNodeSize();
      this.updateContentItemsSize();
    } finally {
      this.layoutManager.endVirtualSizedContainerAdding();
    }
  }
  init() {
    if (this.isInitialised === true)
      return;
    this.updateNodeSize();
    for (let i = 0; i < this.contentItems.length; i++) {
      this._childElementContainer.appendChild(this.contentItems[i].element);
    }
    super.init();
    for (let i = 0; i < this.contentItems.length - 1; i++) {
      this.contentItems[i].element.insertAdjacentElement("afterend", this.createSplitter(i).element);
    }
    this.initContentItems();
  }
  toConfig() {
    const result = {
      type: this.type,
      content: this.calculateConfigContent(),
      width: this.width,
      minWidth: this.minWidth,
      height: this.height,
      minHeight: this.minHeight,
      id: this.id,
      isClosable: this.isClosable
    };
    return result;
  }
  setParent(parent) {
    this._rowOrColumnParent = parent;
    super.setParent(parent);
  }
  updateNodeSize() {
    if (this.contentItems.length > 0) {
      this.calculateRelativeSizes();
      this.setAbsoluteSizes();
    }
    this.emitBaseBubblingEvent("stateChanged");
    this.emit("resize");
  }
  setAbsoluteSizes() {
    const sizeData = this.calculateAbsoluteSizes();
    for (let i = 0; i < this.contentItems.length; i++) {
      if (sizeData.additionalPixel - i > 0) {
        sizeData.itemSizes[i]++;
      }
      if (this._isColumn) {
        setElementWidth(this.contentItems[i].element, sizeData.totalWidth);
        setElementHeight(this.contentItems[i].element, sizeData.itemSizes[i]);
      } else {
        setElementWidth(this.contentItems[i].element, sizeData.itemSizes[i]);
        setElementHeight(this.contentItems[i].element, sizeData.totalHeight);
      }
    }
  }
  calculateAbsoluteSizes() {
    const totalSplitterSize = (this.contentItems.length - 1) * this._splitterSize;
    let {width: totalWidth, height: totalHeight} = getElementWidthAndHeight(this.element);
    if (this._isColumn) {
      totalHeight -= totalSplitterSize;
    } else {
      totalWidth -= totalSplitterSize;
    }
    let totalAssigned = 0;
    const itemSizes = [];
    for (let i = 0; i < this.contentItems.length; i++) {
      let itemSize;
      if (this._isColumn) {
        itemSize = Math.floor(totalHeight * (this.contentItems[i].height / 100));
      } else {
        itemSize = Math.floor(totalWidth * (this.contentItems[i].width / 100));
      }
      totalAssigned += itemSize;
      itemSizes.push(itemSize);
    }
    const additionalPixel = Math.floor((this._isColumn ? totalHeight : totalWidth) - totalAssigned);
    return {
      itemSizes,
      additionalPixel,
      totalWidth,
      totalHeight
    };
  }
  calculateRelativeSizes() {
    let total = 0;
    const itemsWithoutSetDimension = [];
    for (let i = 0; i < this.contentItems.length; i++) {
      if (this.contentItems[i][this._dimension] !== void 0) {
        total += this.contentItems[i][this._dimension];
      } else {
        itemsWithoutSetDimension.push(this.contentItems[i]);
      }
    }
    if (Math.round(total) === 100) {
      this.respectMinItemWidth();
      return;
    }
    if (Math.round(total) < 100 && itemsWithoutSetDimension.length > 0) {
      for (let i = 0; i < itemsWithoutSetDimension.length; i++) {
        itemsWithoutSetDimension[i][this._dimension] = (100 - total) / itemsWithoutSetDimension.length;
      }
      this.respectMinItemWidth();
      return;
    }
    if (Math.round(total) > 100) {
      for (let i = 0; i < itemsWithoutSetDimension.length; i++) {
        itemsWithoutSetDimension[i][this._dimension] = 50;
        total += 50;
      }
    }
    for (let i = 0; i < this.contentItems.length; i++) {
      this.contentItems[i][this._dimension] = this.contentItems[i][this._dimension] / total * 100;
    }
    this.respectMinItemWidth();
  }
  respectMinItemWidth() {
    const minItemWidth = this.layoutManager.layoutConfig.dimensions.minItemWidth;
    let totalOverMin = 0;
    let totalUnderMin = 0;
    const entriesOverMin = [];
    const allEntries = [];
    if (this._isColumn || !minItemWidth || this.contentItems.length <= 1) {
      return;
    }
    const sizeData = this.calculateAbsoluteSizes();
    for (let i = 0; i < sizeData.itemSizes.length; i++) {
      const itemSize = sizeData.itemSizes[i];
      let entry;
      if (itemSize < minItemWidth) {
        totalUnderMin += minItemWidth - itemSize;
        entry = {
          width: minItemWidth
        };
      } else {
        totalOverMin += itemSize - minItemWidth;
        entry = {
          width: itemSize
        };
        entriesOverMin.push(entry);
      }
      allEntries.push(entry);
    }
    if (totalUnderMin === 0 || totalUnderMin > totalOverMin) {
      return;
    }
    const reducePercent = totalUnderMin / totalOverMin;
    let remainingWidth = totalUnderMin;
    for (let i = 0; i < entriesOverMin.length; i++) {
      const entry = entriesOverMin[i];
      const reducedWidth = Math.round((entry.width - minItemWidth) * reducePercent);
      remainingWidth -= reducedWidth;
      entry.width -= reducedWidth;
    }
    if (remainingWidth !== 0) {
      allEntries[allEntries.length - 1].width -= remainingWidth;
    }
    for (let i = 0; i < this.contentItems.length; i++) {
      this.contentItems[i].width = allEntries[i].width / sizeData.totalWidth * 100;
    }
  }
  createSplitter(index) {
    const splitter = new Splitter(this._isColumn, this._splitterSize, this._splitterGrabSize);
    splitter.on("drag", (offsetX, offsetY) => this.onSplitterDrag(splitter, offsetX, offsetY));
    splitter.on("dragStop", () => this.onSplitterDragStop(splitter));
    splitter.on("dragStart", () => this.onSplitterDragStart(splitter));
    this._splitter.splice(index, 0, splitter);
    return splitter;
  }
  getItemsForSplitter(splitter) {
    const index = this._splitter.indexOf(splitter);
    return {
      before: this.contentItems[index],
      after: this.contentItems[index + 1]
    };
  }
  getMinimumDimensions(arr) {
    var _a, _b;
    let minWidth = 0;
    let minHeight = 0;
    for (let i = 0; i < arr.length; ++i) {
      minWidth = Math.max((_a = arr[i].minWidth) !== null && _a !== void 0 ? _a : 0, minWidth);
      minHeight = Math.max((_b = arr[i].minHeight) !== null && _b !== void 0 ? _b : 0, minHeight);
    }
    return {
      horizontal: minWidth,
      vertical: minHeight
    };
  }
  onSplitterDragStart(splitter) {
    const items = this.getItemsForSplitter(splitter);
    const minSize = this.layoutManager.layoutConfig.dimensions[this._isColumn ? "minItemHeight" : "minItemWidth"];
    const beforeMinDim = this.getMinimumDimensions(items.before.contentItems);
    const beforeMinSize = this._isColumn ? beforeMinDim.vertical : beforeMinDim.horizontal;
    const afterMinDim = this.getMinimumDimensions(items.after.contentItems);
    const afterMinSize = this._isColumn ? afterMinDim.vertical : afterMinDim.horizontal;
    this._splitterPosition = 0;
    this._splitterMinPosition = -1 * (pixelsToNumber(items.before.element.style[this._dimension]) - (beforeMinSize || minSize));
    this._splitterMaxPosition = pixelsToNumber(items.after.element.style[this._dimension]) - (afterMinSize || minSize);
  }
  onSplitterDrag(splitter, offsetX, offsetY) {
    let offset = this._isColumn ? offsetY : offsetX;
    if (this._splitterMinPosition === null || this._splitterMaxPosition === null) {
      throw new UnexpectedNullError("ROCOSD59226");
    }
    offset = Math.max(offset, this._splitterMinPosition);
    offset = Math.min(offset, this._splitterMaxPosition);
    this._splitterPosition = offset;
    const offsetPixels = numberToPixels(offset);
    if (this._isColumn) {
      splitter.element.style.top = offsetPixels;
    } else {
      splitter.element.style.left = offsetPixels;
    }
  }
  onSplitterDragStop(splitter) {
    if (this._splitterPosition === null) {
      throw new UnexpectedNullError("ROCOSDS66932");
    } else {
      const items = this.getItemsForSplitter(splitter);
      const sizeBefore = pixelsToNumber(items.before.element.style[this._dimension]);
      const sizeAfter = pixelsToNumber(items.after.element.style[this._dimension]);
      const splitterPositionInRange = (this._splitterPosition + sizeBefore) / (sizeBefore + sizeAfter);
      const totalRelativeSize = items.before[this._dimension] + items.after[this._dimension];
      items.before[this._dimension] = splitterPositionInRange * totalRelativeSize;
      items.after[this._dimension] = (1 - splitterPositionInRange) * totalRelativeSize;
      splitter.element.style.top = numberToPixels(0);
      splitter.element.style.left = numberToPixels(0);
      globalThis.requestAnimationFrame(() => this.updateSize());
    }
  }
}
(function(RowOrColumn2) {
  function getElementDimensionSize(element, dimension) {
    if (dimension === "width") {
      return getElementWidth(element);
    } else {
      return getElementHeight(element);
    }
  }
  RowOrColumn2.getElementDimensionSize = getElementDimensionSize;
  function setElementDimensionSize(element, dimension, value) {
    if (dimension === "width") {
      return setElementWidth(element, value);
    } else {
      return setElementHeight(element, value);
    }
  }
  RowOrColumn2.setElementDimensionSize = setElementDimensionSize;
  function createElement(document2, isColumn) {
    const element = document2.createElement("div");
    element.classList.add("lm_item");
    if (isColumn) {
      element.classList.add("lm_column");
    } else {
      element.classList.add("lm_row");
    }
    return element;
  }
  RowOrColumn2.createElement = createElement;
})(RowOrColumn || (RowOrColumn = {}));
class GroundItem extends ComponentParentableItem {
  constructor(layoutManager, rootItemConfig, containerElement) {
    super(layoutManager, ResolvedGroundItemConfig.create(rootItemConfig), null, GroundItem.createElement(document));
    this.isGround = true;
    this._childElementContainer = this.element;
    this._containerElement = containerElement;
    this._containerElement.appendChild(this.element);
  }
  init() {
    if (this.isInitialised === true)
      return;
    this.updateNodeSize();
    for (let i = 0; i < this.contentItems.length; i++) {
      this._childElementContainer.appendChild(this.contentItems[i].element);
    }
    super.init();
    this.initContentItems();
  }
  loadRoot(rootItemConfig) {
    this.clearRoot();
    if (rootItemConfig !== void 0) {
      const rootContentItem = this.layoutManager.createAndInitContentItem(rootItemConfig, this);
      this.addChild(rootContentItem, 0);
    }
  }
  clearRoot() {
    const contentItems = this.contentItems;
    switch (contentItems.length) {
      case 0: {
        return;
      }
      case 1: {
        const existingRootContentItem = contentItems[0];
        existingRootContentItem.remove();
        return;
      }
      default: {
        throw new AssertError("GILR07721");
      }
    }
  }
  addItem(itemConfig, index) {
    this.layoutManager.checkMinimiseMaximisedStack();
    const resolvedItemConfig = ItemConfig.resolve(itemConfig);
    let parent;
    if (this.contentItems.length > 0) {
      parent = this.contentItems[0];
    } else {
      parent = this;
    }
    if (parent.isComponent) {
      throw new Error("Cannot add item as child to ComponentItem");
    } else {
      const contentItem = this.layoutManager.createAndInitContentItem(resolvedItemConfig, parent);
      index = parent.addChild(contentItem, index);
      return parent === this ? -1 : index;
    }
  }
  loadComponentAsRoot(itemConfig) {
    this.clearRoot();
    const resolvedItemConfig = ItemConfig.resolve(itemConfig);
    if (resolvedItemConfig.maximised) {
      throw new Error("Root Component cannot be maximised");
    } else {
      const rootContentItem = new ComponentItem(this.layoutManager, resolvedItemConfig, this);
      rootContentItem.init();
      this.addChild(rootContentItem, 0);
    }
  }
  addChild(contentItem, index) {
    if (this.contentItems.length > 0) {
      throw new Error("Ground node can only have a single child");
    } else {
      this._childElementContainer.appendChild(contentItem.element);
      index = super.addChild(contentItem, index);
      this.updateSize();
      this.emitBaseBubblingEvent("stateChanged");
      return index;
    }
  }
  calculateConfigContent() {
    const contentItems = this.contentItems;
    const count = contentItems.length;
    const result = new Array(count);
    for (let i = 0; i < count; i++) {
      const item = contentItems[i];
      const itemConfig = item.toConfig();
      if (ResolvedRootItemConfig.isRootItemConfig(itemConfig)) {
        result[i] = itemConfig;
      } else {
        throw new AssertError("RCCC66832");
      }
    }
    return result;
  }
  setSize(width, height) {
    if (width === void 0 || height === void 0) {
      this.updateSize();
    } else {
      setElementWidth(this.element, width);
      setElementHeight(this.element, height);
      if (this.contentItems.length > 0) {
        setElementWidth(this.contentItems[0].element, width);
        setElementHeight(this.contentItems[0].element, height);
      }
      this.updateContentItemsSize();
    }
  }
  updateSize() {
    this.layoutManager.beginVirtualSizedContainerAdding();
    try {
      this.updateNodeSize();
      this.updateContentItemsSize();
    } finally {
      this.layoutManager.endVirtualSizedContainerAdding();
    }
  }
  createSideAreas() {
    const areaSize = 50;
    const oppositeSides = GroundItem.Area.oppositeSides;
    const result = new Array(Object.keys(oppositeSides).length);
    let idx = 0;
    for (const key in oppositeSides) {
      const side = key;
      const area = this.getElementArea();
      if (area === null) {
        throw new UnexpectedNullError("RCSA77553");
      } else {
        area.side = side;
        if (oppositeSides[side][1] === "2")
          area[side] = area[oppositeSides[side]] - areaSize;
        else
          area[side] = area[oppositeSides[side]] + areaSize;
        area.surface = (area.x2 - area.x1) * (area.y2 - area.y1);
        result[idx++] = area;
      }
    }
    return result;
  }
  highlightDropZone(x, y, area) {
    this.layoutManager.tabDropPlaceholder.remove();
    super.highlightDropZone(x, y, area);
  }
  onDrop(contentItem, area) {
    if (contentItem.isComponent) {
      const itemConfig = ResolvedStackItemConfig.createDefault();
      const component = contentItem;
      itemConfig.header = ResolvedHeaderedItemConfig.Header.createCopy(component.headerConfig);
      const stack = this.layoutManager.createAndInitContentItem(itemConfig, this);
      stack.addChild(contentItem);
      contentItem = stack;
    }
    if (this.contentItems.length === 0) {
      this.addChild(contentItem);
    } else {
      if (contentItem.type === ItemType.row || contentItem.type === ItemType.column) {
        const itemConfig = ResolvedStackItemConfig.createDefault();
        const stack = this.layoutManager.createContentItem(itemConfig, this);
        stack.addChild(contentItem);
        contentItem = stack;
      }
      const type = area.side[0] == "x" ? ItemType.row : ItemType.column;
      const dimension = area.side[0] == "x" ? "width" : "height";
      const insertBefore = area.side[1] == "2";
      const column = this.contentItems[0];
      if (!(column instanceof RowOrColumn) || column.type !== type) {
        const itemConfig = ResolvedItemConfig.createDefault(type);
        const rowOrColumn = this.layoutManager.createContentItem(itemConfig, this);
        this.replaceChild(column, rowOrColumn);
        rowOrColumn.addChild(contentItem, insertBefore ? 0 : void 0, true);
        rowOrColumn.addChild(column, insertBefore ? void 0 : 0, true);
        column[dimension] = 50;
        contentItem[dimension] = 50;
        rowOrColumn.updateSize();
      } else {
        const sibling = column.contentItems[insertBefore ? 0 : column.contentItems.length - 1];
        column.addChild(contentItem, insertBefore ? 0 : void 0, true);
        sibling[dimension] *= 0.5;
        contentItem[dimension] = sibling[dimension];
        column.updateSize();
      }
    }
  }
  dock() {
    throw new AssertError("GID87731");
  }
  validateDocking() {
    throw new AssertError("GIVD87732");
  }
  getAllContentItems() {
    const result = [this];
    this.deepGetAllContentItems(this.contentItems, result);
    return result;
  }
  getConfigMaximisedItems() {
    const result = [];
    this.deepFilterContentItems(this.contentItems, result, (item) => {
      if (ContentItem.isStack(item) && item.initialWantMaximise) {
        return true;
      } else {
        if (ContentItem.isComponentItem(item) && item.initialWantMaximise) {
          return true;
        } else {
          return false;
        }
      }
    });
    return result;
  }
  getItemsByPopInParentId(popInParentId) {
    const result = [];
    this.deepFilterContentItems(this.contentItems, result, (item) => item.popInParentIds.includes(popInParentId));
    return result;
  }
  toConfig() {
    throw new Error("Cannot generate GroundItem config");
  }
  setActiveComponentItem(item, focus, suppressFocusEvent) {
  }
  updateNodeSize() {
    const {width, height} = getElementWidthAndHeight(this._containerElement);
    setElementWidth(this.element, width);
    setElementHeight(this.element, height);
    if (this.contentItems.length > 0) {
      setElementWidth(this.contentItems[0].element, width);
      setElementHeight(this.contentItems[0].element, height);
    }
  }
  deepGetAllContentItems(content, result) {
    for (let i = 0; i < content.length; i++) {
      const contentItem = content[i];
      result.push(contentItem);
      this.deepGetAllContentItems(contentItem.contentItems, result);
    }
  }
  deepFilterContentItems(content, result, checkAcceptFtn) {
    for (let i = 0; i < content.length; i++) {
      const contentItem = content[i];
      if (checkAcceptFtn(contentItem)) {
        result.push(contentItem);
      }
      this.deepFilterContentItems(contentItem.contentItems, result, checkAcceptFtn);
    }
  }
}
(function(GroundItem2) {
  (function(Area) {
    Area.oppositeSides = {
      y2: "y1",
      x2: "x1",
      y1: "y2",
      x1: "x2"
    };
  })(GroundItem2.Area || (GroundItem2.Area = {}));
  function createElement(document2) {
    const element = document2.createElement("div");
    element.classList.add("lm_goldenlayout");
    element.classList.add("lm_item");
    element.classList.add("lm_root");
    return element;
  }
  GroundItem2.createElement = createElement;
})(GroundItem || (GroundItem = {}));
class HeaderButton {
  constructor(_header, label, cssClass, _pushEvent) {
    this._header = _header;
    this._pushEvent = _pushEvent;
    this._clickEventListener = (ev) => this.onClick(ev);
    this._touchStartEventListener = (ev) => this.onTouchStart(ev);
    this._element = document.createElement("div");
    this._element.classList.add(cssClass);
    this._element.title = label;
    this._header.on("destroy", () => this.destroy());
    this._element.addEventListener("click", this._clickEventListener, {passive: true});
    this._element.addEventListener("touchstart", this._touchStartEventListener, {passive: true});
    this._header.controlsContainerElement.appendChild(this._element);
  }
  get element() {
    return this._element;
  }
  destroy() {
    var _a;
    this._element.removeEventListener("click", this._clickEventListener);
    this._element.removeEventListener("touchstart", this._touchStartEventListener);
    (_a = this._element.parentNode) === null || _a === void 0 ? void 0 : _a.removeChild(this._element);
  }
  onClick(ev) {
    this._pushEvent(ev);
  }
  onTouchStart(ev) {
    this._pushEvent(ev);
  }
}
class Tab {
  constructor(_layoutManager, _componentItem, _closeEvent, _focusEvent, _dragStartEvent) {
    var _a;
    this._layoutManager = _layoutManager;
    this._componentItem = _componentItem;
    this._closeEvent = _closeEvent;
    this._focusEvent = _focusEvent;
    this._dragStartEvent = _dragStartEvent;
    this._isActive = false;
    this._tabClickListener = (ev) => this.onTabClickDown(ev);
    this._tabTouchStartListener = (ev) => this.onTabTouchStart(ev);
    this._closeClickListener = () => this.onCloseClick();
    this._closeTouchStartListener = () => this.onCloseTouchStart();
    this._dragStartListener = (x, y) => this.onDragStart(x, y);
    this._contentItemDestroyListener = () => this.onContentItemDestroy();
    this._tabTitleChangedListener = (title) => this.setTitle(title);
    this._element = document.createElement("div");
    this._element.classList.add("lm_tab");
    this._titleElement = document.createElement("span");
    this._titleElement.classList.add("lm_title");
    this._closeElement = document.createElement("div");
    this._closeElement.classList.add("lm_close_tab");
    this._element.appendChild(this._titleElement);
    this._element.appendChild(this._closeElement);
    if (_componentItem.isClosable) {
      this._closeElement.style.display = "";
    } else {
      this._closeElement.style.display = "none";
    }
    this.setTitle(_componentItem.title);
    this._componentItem.on("titleChanged", this._tabTitleChangedListener);
    const reorderEnabled = (_a = _componentItem.reorderEnabled) !== null && _a !== void 0 ? _a : this._layoutManager.layoutConfig.settings.reorderEnabled;
    if (reorderEnabled) {
      this.enableReorder();
    }
    this._element.addEventListener("click", this._tabClickListener, {passive: true});
    this._element.addEventListener("touchstart", this._tabTouchStartListener, {passive: true});
    if (this._componentItem.isClosable) {
      this._closeElement.addEventListener("click", this._closeClickListener, {passive: true});
      this._closeElement.addEventListener("touchstart", this._closeTouchStartListener, {passive: true});
    } else {
      this._closeElement.remove();
      this._closeElement = void 0;
    }
    this._componentItem.setTab(this);
    this._layoutManager.emit("tabCreated", this);
  }
  get isActive() {
    return this._isActive;
  }
  get componentItem() {
    return this._componentItem;
  }
  get contentItem() {
    return this._componentItem;
  }
  get element() {
    return this._element;
  }
  get titleElement() {
    return this._titleElement;
  }
  get closeElement() {
    return this._closeElement;
  }
  get reorderEnabled() {
    return this._dragListener !== void 0;
  }
  set reorderEnabled(value) {
    if (value !== this.reorderEnabled) {
      if (value) {
        this.enableReorder();
      } else {
        this.disableReorder();
      }
    }
  }
  setTitle(title) {
    this._titleElement.innerText = title;
    this._element.title = title;
  }
  setActive(isActive) {
    if (isActive === this._isActive) {
      return;
    }
    this._isActive = isActive;
    if (isActive) {
      this._element.classList.add("lm_active");
    } else {
      this._element.classList.remove("lm_active");
    }
  }
  destroy() {
    var _a, _b;
    this._closeEvent = void 0;
    this._focusEvent = void 0;
    this._dragStartEvent = void 0;
    this._element.removeEventListener("click", this._tabClickListener);
    this._element.removeEventListener("touchstart", this._tabTouchStartListener);
    (_a = this._closeElement) === null || _a === void 0 ? void 0 : _a.removeEventListener("click", this._closeClickListener);
    (_b = this._closeElement) === null || _b === void 0 ? void 0 : _b.removeEventListener("touchstart", this._closeTouchStartListener);
    this._componentItem.off("titleChanged", this._tabTitleChangedListener);
    if (this.reorderEnabled) {
      this.disableReorder();
    }
    this._element.remove();
  }
  setBlurred() {
    this._element.classList.remove("lm_focused");
    this._titleElement.classList.remove("lm_focused");
  }
  setFocused() {
    this._element.classList.add("lm_focused");
    this._titleElement.classList.add("lm_focused");
  }
  onDragStart(x, y) {
    if (this._dragListener === void 0) {
      throw new UnexpectedUndefinedError("TODSDLU10093");
    } else {
      if (this._dragStartEvent === void 0) {
        throw new UnexpectedUndefinedError("TODS23309");
      } else {
        this._dragStartEvent(x, y, this._dragListener, this.componentItem);
      }
    }
  }
  onContentItemDestroy() {
    if (this._dragListener !== void 0) {
      this._dragListener.destroy();
      this._dragListener = void 0;
    }
  }
  onTabClickDown(event) {
    const target = event.target;
    if (target === this._element || target === this._titleElement) {
      if (event.button === 0) {
        this.notifyFocus();
      } else if (event.button === 1 && this._componentItem.isClosable) {
        this.notifyClose();
      }
    }
  }
  onTabTouchStart(event) {
    if (event.target === this._element) {
      this.notifyFocus();
    }
  }
  onCloseClick() {
    this.notifyClose();
  }
  onCloseTouchStart() {
    this.notifyClose();
  }
  notifyClose() {
    if (this._closeEvent === void 0) {
      throw new UnexpectedUndefinedError("TNC15007");
    } else {
      this._closeEvent(this._componentItem);
    }
  }
  notifyFocus() {
    if (this._focusEvent === void 0) {
      throw new UnexpectedUndefinedError("TNA15007");
    } else {
      this._focusEvent(this._componentItem);
    }
  }
  enableReorder() {
    this._dragListener = new DragListener(this._element, [this._titleElement]);
    this._dragListener.on("dragStart", this._dragStartListener);
    this._componentItem.on("destroy", this._contentItemDestroyListener);
  }
  disableReorder() {
    if (this._dragListener === void 0) {
      throw new UnexpectedUndefinedError("TDR87745");
    } else {
      this._componentItem.off("destroy", this._contentItemDestroyListener);
      this._dragListener.off("dragStart", this._dragStartListener);
      this._dragListener = void 0;
    }
  }
}
class TabsContainer {
  constructor(_layoutManager, _componentRemoveEvent, _componentFocusEvent, _componentDragStartEvent, _dropdownActiveChangedEvent) {
    this._layoutManager = _layoutManager;
    this._componentRemoveEvent = _componentRemoveEvent;
    this._componentFocusEvent = _componentFocusEvent;
    this._componentDragStartEvent = _componentDragStartEvent;
    this._dropdownActiveChangedEvent = _dropdownActiveChangedEvent;
    this._tabs = [];
    this._lastVisibleTabIndex = -1;
    this._dropdownActive = false;
    this._element = document.createElement("section");
    this._element.classList.add("lm_tabs");
    this._dropdownElement = document.createElement("section");
    this._dropdownElement.classList.add("lm_tabdropdown_list");
    this._dropdownElement.style.display = "none";
  }
  get tabs() {
    return this._tabs;
  }
  get tabCount() {
    return this._tabs.length;
  }
  get lastVisibleTabIndex() {
    return this._lastVisibleTabIndex;
  }
  get element() {
    return this._element;
  }
  get dropdownElement() {
    return this._dropdownElement;
  }
  get dropdownActive() {
    return this._dropdownActive;
  }
  destroy() {
    for (let i = 0; i < this._tabs.length; i++) {
      this._tabs[i].destroy();
    }
  }
  createTab(componentItem, index) {
    for (let i = 0; i < this._tabs.length; i++) {
      if (this._tabs[i].componentItem === componentItem) {
        return;
      }
    }
    const tab = new Tab(this._layoutManager, componentItem, (item) => this.handleTabCloseEvent(item), (item) => this.handleTabFocusEvent(item), (x, y, dragListener, item) => this.handleTabDragStartEvent(x, y, dragListener, item));
    if (this._tabs.length === 0) {
      this._tabs.push(tab);
      this._element.appendChild(tab.element);
    } else {
      if (index === void 0) {
        index = this._tabs.length;
      }
      if (index > 0) {
        this._tabs[index - 1].element.insertAdjacentElement("afterend", tab.element);
      } else {
        this._tabs[0].element.insertAdjacentElement("beforebegin", tab.element);
      }
      this._tabs.splice(index, 0, tab);
    }
  }
  removeTab(componentItem) {
    for (let i = 0; i < this._tabs.length; i++) {
      if (this._tabs[i].componentItem === componentItem) {
        const tab = this._tabs[i];
        tab.destroy();
        this._tabs.splice(i, 1);
        return;
      }
    }
    throw new Error("contentItem is not controlled by this header");
  }
  processActiveComponentChanged(newActiveComponentItem) {
    let activeIndex = -1;
    for (let i = 0; i < this._tabs.length; i++) {
      const isActive = this._tabs[i].componentItem === newActiveComponentItem;
      this._tabs[i].setActive(isActive);
      if (isActive) {
        activeIndex = i;
      }
    }
    if (activeIndex < 0) {
      throw new AssertError("HSACI56632");
    } else {
      if (this._layoutManager.layoutConfig.settings.reorderOnTabMenuClick) {
        if (this._lastVisibleTabIndex !== -1 && activeIndex > this._lastVisibleTabIndex) {
          const activeTab = this._tabs[activeIndex];
          for (let j = activeIndex; j > 0; j--) {
            this._tabs[j] = this._tabs[j - 1];
          }
          this._tabs[0] = activeTab;
        }
      }
    }
  }
  updateTabSizes(availableWidth, activeComponentItem) {
    let dropDownActive = false;
    const success = this.tryUpdateTabSizes(dropDownActive, availableWidth, activeComponentItem);
    if (!success) {
      dropDownActive = true;
      this.tryUpdateTabSizes(dropDownActive, availableWidth, activeComponentItem);
    }
    if (dropDownActive !== this._dropdownActive) {
      this._dropdownActive = dropDownActive;
      this._dropdownActiveChangedEvent();
    }
  }
  tryUpdateTabSizes(dropdownActive, availableWidth, activeComponentItem) {
    if (this._tabs.length > 0) {
      if (activeComponentItem === void 0) {
        throw new Error("non-empty tabs must have active component item");
      }
      let cumulativeTabWidth = 0;
      let tabOverlapAllowanceExceeded = false;
      const tabOverlapAllowance = this._layoutManager.layoutConfig.settings.tabOverlapAllowance;
      const activeIndex = this._tabs.indexOf(activeComponentItem.tab);
      const activeTab = this._tabs[activeIndex];
      this._lastVisibleTabIndex = -1;
      for (let i = 0; i < this._tabs.length; i++) {
        const tabElement = this._tabs[i].element;
        if (tabElement.parentElement !== this._element) {
          this._element.appendChild(tabElement);
        }
        const tabMarginRightPixels = getComputedStyle(activeTab.element).marginRight;
        const tabMarginRight = pixelsToNumber(tabMarginRightPixels);
        const tabWidth = tabElement.offsetWidth + tabMarginRight;
        cumulativeTabWidth += tabWidth;
        let visibleTabWidth = 0;
        if (activeIndex <= i) {
          visibleTabWidth = cumulativeTabWidth;
        } else {
          const activeTabMarginRightPixels = getComputedStyle(activeTab.element).marginRight;
          const activeTabMarginRight = pixelsToNumber(activeTabMarginRightPixels);
          visibleTabWidth = cumulativeTabWidth + activeTab.element.offsetWidth + activeTabMarginRight;
        }
        if (visibleTabWidth > availableWidth) {
          if (!tabOverlapAllowanceExceeded) {
            let overlap;
            if (activeIndex > 0 && activeIndex <= i) {
              overlap = (visibleTabWidth - availableWidth) / (i - 1);
            } else {
              overlap = (visibleTabWidth - availableWidth) / i;
            }
            if (overlap < tabOverlapAllowance) {
              for (let j = 0; j <= i; j++) {
                const marginLeft = j !== activeIndex && j !== 0 ? "-" + numberToPixels(overlap) : "";
                this._tabs[j].element.style.zIndex = numberToPixels(i - j);
                this._tabs[j].element.style.marginLeft = marginLeft;
              }
              this._lastVisibleTabIndex = i;
              if (tabElement.parentElement !== this._element) {
                this._element.appendChild(tabElement);
              }
            } else {
              tabOverlapAllowanceExceeded = true;
            }
          } else if (i === activeIndex) {
            tabElement.style.zIndex = "auto";
            tabElement.style.marginLeft = "";
            if (tabElement.parentElement !== this._element) {
              this._element.appendChild(tabElement);
            }
          }
          if (tabOverlapAllowanceExceeded && i !== activeIndex) {
            if (dropdownActive) {
              tabElement.style.zIndex = "auto";
              tabElement.style.marginLeft = "";
              if (tabElement.parentElement !== this._dropdownElement) {
                this._dropdownElement.appendChild(tabElement);
              }
            } else {
              return false;
            }
          }
        } else {
          this._lastVisibleTabIndex = i;
          tabElement.style.zIndex = "auto";
          tabElement.style.marginLeft = "";
          if (tabElement.parentElement !== this._element) {
            this._element.appendChild(tabElement);
          }
        }
      }
    }
    return true;
  }
  showAdditionalTabsDropdown() {
    this._dropdownElement.style.display = "";
  }
  hideAdditionalTabsDropdown() {
    this._dropdownElement.style.display = "none";
  }
  handleTabCloseEvent(componentItem) {
    this._componentRemoveEvent(componentItem);
  }
  handleTabFocusEvent(componentItem) {
    this._componentFocusEvent(componentItem);
  }
  handleTabDragStartEvent(x, y, dragListener, componentItem) {
    this._componentDragStartEvent(x, y, dragListener, componentItem);
  }
}
class Header extends EventEmitter {
  constructor(_layoutManager, _parent, settings, _configClosable, _getActiveComponentItemEvent, closeEvent, _popoutEvent, _maximiseToggleEvent, _clickEvent, _touchStartEvent, _componentRemoveEvent, _componentFocusEvent, _componentDragStartEvent) {
    super();
    this._layoutManager = _layoutManager;
    this._parent = _parent;
    this._configClosable = _configClosable;
    this._getActiveComponentItemEvent = _getActiveComponentItemEvent;
    this._popoutEvent = _popoutEvent;
    this._maximiseToggleEvent = _maximiseToggleEvent;
    this._clickEvent = _clickEvent;
    this._touchStartEvent = _touchStartEvent;
    this._componentRemoveEvent = _componentRemoveEvent;
    this._componentFocusEvent = _componentFocusEvent;
    this._componentDragStartEvent = _componentDragStartEvent;
    this._clickListener = (ev) => this.onClick(ev);
    this._touchStartListener = (ev) => this.onTouchStart(ev);
    this._rowColumnClosable = true;
    this._closeButton = null;
    this._popoutButton = null;
    this._tabsContainer = new TabsContainer(this._layoutManager, (item) => this.handleTabInitiatedComponentRemoveEvent(item), (item) => this.handleTabInitiatedComponentFocusEvent(item), (x, y, dragListener, item) => this.handleTabInitiatedDragStartEvent(x, y, dragListener, item), () => this.processTabDropdownActiveChanged());
    this._show = settings.show;
    this._popoutEnabled = settings.popoutEnabled;
    this._popoutLabel = settings.popoutLabel;
    this._maximiseEnabled = settings.maximiseEnabled;
    this._maximiseLabel = settings.maximiseLabel;
    this._minimiseEnabled = settings.minimiseEnabled;
    this._minimiseLabel = settings.minimiseLabel;
    this._closeEnabled = settings.closeEnabled;
    this._closeLabel = settings.closeLabel;
    this._tabDropdownEnabled = settings.tabDropdownEnabled;
    this._tabDropdownLabel = settings.tabDropdownLabel;
    this.setSide(settings.side);
    this._canRemoveComponent = this._configClosable;
    this._element = document.createElement("section");
    this._element.classList.add("lm_header");
    this._controlsContainerElement = document.createElement("section");
    this._controlsContainerElement.classList.add("lm_controls");
    this._element.appendChild(this._tabsContainer.element);
    this._element.appendChild(this._controlsContainerElement);
    this._element.appendChild(this._tabsContainer.dropdownElement);
    this._element.addEventListener("click", this._clickListener, {passive: true});
    this._element.addEventListener("touchstart", this._touchStartListener, {passive: true});
    this._documentMouseUpListener = () => this._tabsContainer.hideAdditionalTabsDropdown();
    globalThis.document.addEventListener("mouseup", this._documentMouseUpListener, {passive: true});
    this._tabControlOffset = this._layoutManager.layoutConfig.settings.tabControlOffset;
    if (this._tabDropdownEnabled) {
      this._tabDropdownButton = new HeaderButton(this, this._tabDropdownLabel, "lm_tabdropdown", () => this._tabsContainer.showAdditionalTabsDropdown());
    }
    if (this._popoutEnabled) {
      this._popoutButton = new HeaderButton(this, this._popoutLabel, "lm_popout", () => this.handleButtonPopoutEvent());
    }
    if (this._maximiseEnabled) {
      this._maximiseButton = new HeaderButton(this, this._maximiseLabel, "lm_maximise", (ev) => this.handleButtonMaximiseToggleEvent(ev));
    }
    if (this._configClosable) {
      this._closeButton = new HeaderButton(this, this._closeLabel, "lm_close", () => closeEvent());
    }
    this.processTabDropdownActiveChanged();
  }
  get show() {
    return this._show;
  }
  get side() {
    return this._side;
  }
  get leftRightSided() {
    return this._leftRightSided;
  }
  get layoutManager() {
    return this._layoutManager;
  }
  get parent() {
    return this._parent;
  }
  get tabs() {
    return this._tabsContainer.tabs;
  }
  get lastVisibleTabIndex() {
    return this._tabsContainer.lastVisibleTabIndex;
  }
  get element() {
    return this._element;
  }
  get tabsContainerElement() {
    return this._tabsContainer.element;
  }
  get controlsContainerElement() {
    return this._controlsContainerElement;
  }
  destroy() {
    this.emit("destroy");
    this._popoutEvent = void 0;
    this._maximiseToggleEvent = void 0;
    this._clickEvent = void 0;
    this._touchStartEvent = void 0;
    this._componentRemoveEvent = void 0;
    this._componentFocusEvent = void 0;
    this._componentDragStartEvent = void 0;
    this._tabsContainer.destroy();
    globalThis.document.removeEventListener("mouseup", this._documentMouseUpListener);
    this._element.remove();
  }
  createTab(componentItem, index) {
    this._tabsContainer.createTab(componentItem, index);
  }
  removeTab(componentItem) {
    this._tabsContainer.removeTab(componentItem);
  }
  processActiveComponentChanged(newActiveComponentItem) {
    this._tabsContainer.processActiveComponentChanged(newActiveComponentItem);
    this.updateTabSizes();
  }
  setSide(value) {
    this._side = value;
    this._leftRightSided = [Side.right, Side.left].includes(this._side);
  }
  setRowColumnClosable(value) {
    this._rowColumnClosable = value;
    this.updateClosability();
  }
  updateClosability() {
    let isClosable;
    if (!this._configClosable) {
      isClosable = false;
    } else {
      if (!this._rowColumnClosable) {
        isClosable = false;
      } else {
        isClosable = true;
        const len = this.tabs.length;
        for (let i = 0; i < len; i++) {
          const tab = this._tabsContainer.tabs[i];
          const item = tab.componentItem;
          if (!item.isClosable) {
            isClosable = false;
            break;
          }
        }
      }
    }
    if (this._closeButton !== null) {
      setElementDisplayVisibility(this._closeButton.element, isClosable);
    }
    if (this._popoutButton !== null) {
      setElementDisplayVisibility(this._popoutButton.element, isClosable);
    }
    this._canRemoveComponent = isClosable || this._tabsContainer.tabCount > 1;
  }
  applyFocusedValue(value) {
    if (value) {
      this._element.classList.add("lm_focused");
    } else {
      this._element.classList.remove("lm_focused");
    }
  }
  processMaximised() {
    if (this._maximiseButton === void 0) {
      throw new UnexpectedUndefinedError("HPMAX16997");
    } else {
      this._maximiseButton.element.setAttribute("title", this._minimiseLabel);
    }
  }
  processMinimised() {
    if (this._maximiseButton === void 0) {
      throw new UnexpectedUndefinedError("HPMIN16997");
    } else {
      this._maximiseButton.element.setAttribute("title", this._maximiseLabel);
    }
  }
  updateTabSizes() {
    if (this._tabsContainer.tabCount > 0) {
      const headerHeight = this._show ? this._layoutManager.layoutConfig.dimensions.headerHeight : 0;
      if (this._leftRightSided) {
        this._element.style.height = "";
        this._element.style.width = numberToPixels(headerHeight);
      } else {
        this._element.style.width = "";
        this._element.style.height = numberToPixels(headerHeight);
      }
      let availableWidth;
      if (this._leftRightSided) {
        availableWidth = this._element.offsetHeight - this._controlsContainerElement.offsetHeight - this._tabControlOffset;
      } else {
        availableWidth = this._element.offsetWidth - this._controlsContainerElement.offsetWidth - this._tabControlOffset;
      }
      this._tabsContainer.updateTabSizes(availableWidth, this._getActiveComponentItemEvent());
    }
  }
  handleTabInitiatedComponentRemoveEvent(componentItem) {
    if (this._canRemoveComponent) {
      if (this._componentRemoveEvent === void 0) {
        throw new UnexpectedUndefinedError("HHTCE22294");
      } else {
        this._componentRemoveEvent(componentItem);
      }
    }
  }
  handleTabInitiatedComponentFocusEvent(componentItem) {
    if (this._componentFocusEvent === void 0) {
      throw new UnexpectedUndefinedError("HHTAE22294");
    } else {
      this._componentFocusEvent(componentItem);
    }
  }
  handleTabInitiatedDragStartEvent(x, y, dragListener, componentItem) {
    if (!this._canRemoveComponent) {
      dragListener.cancelDrag();
    } else {
      if (this._componentDragStartEvent === void 0) {
        throw new UnexpectedUndefinedError("HHTDSE22294");
      } else {
        this._componentDragStartEvent(x, y, dragListener, componentItem);
      }
    }
  }
  processTabDropdownActiveChanged() {
    if (this._tabDropdownButton !== void 0) {
      setElementDisplayVisibility(this._tabDropdownButton.element, this._tabsContainer.dropdownActive);
    }
  }
  handleButtonPopoutEvent() {
    if (this._layoutManager.layoutConfig.settings.popoutWholeStack) {
      if (this._popoutEvent === void 0) {
        throw new UnexpectedUndefinedError("HHBPOE17834");
      } else {
        this._popoutEvent();
      }
    } else {
      const activeComponentItem = this._getActiveComponentItemEvent();
      if (activeComponentItem) {
        activeComponentItem.popout();
      }
    }
  }
  handleButtonMaximiseToggleEvent(ev) {
    if (this._maximiseToggleEvent === void 0) {
      throw new UnexpectedUndefinedError("HHBMTE16834");
    } else {
      this._maximiseToggleEvent();
    }
  }
  onClick(event) {
    if (event.target === this._element) {
      this.notifyClick(event);
    }
  }
  onTouchStart(event) {
    if (event.target === this._element) {
      this.notifyTouchStart(event);
    }
  }
  notifyClick(ev) {
    if (this._clickEvent === void 0) {
      throw new UnexpectedUndefinedError("HNHC46834");
    } else {
      this._clickEvent(ev);
    }
  }
  notifyTouchStart(ev) {
    if (this._touchStartEvent === void 0) {
      throw new UnexpectedUndefinedError("HNHTS46834");
    } else {
      this._touchStartEvent(ev);
    }
  }
}
class Stack extends ComponentParentableItem {
  constructor(layoutManager, config, parent) {
    var _a, _b, _c, _d, _e, _f, _g, _h, _j, _k, _l, _m, _o, _p, _q, _r, _s, _t, _u;
    super(layoutManager, config, parent, Stack.createElement(document));
    this._headerSideChanged = false;
    this._resizeListener = () => this.handleResize();
    this._maximisedListener = () => this.handleMaximised();
    this._minimisedListener = () => this.handleMinimised();
    this._headerConfig = config.header;
    const layoutHeaderConfig = layoutManager.layoutConfig.header;
    const configContent = config.content;
    let componentHeaderConfig;
    if (configContent.length !== 1) {
      componentHeaderConfig = void 0;
    } else {
      const firstChildItemConfig = configContent[0];
      componentHeaderConfig = firstChildItemConfig.header;
    }
    this._initialWantMaximise = config.maximised;
    this._initialActiveItemIndex = (_a = config.activeItemIndex) !== null && _a !== void 0 ? _a : 0;
    const show = (_d = (_c = (_b = this._headerConfig) === null || _b === void 0 ? void 0 : _b.show) !== null && _c !== void 0 ? _c : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.show) !== null && _d !== void 0 ? _d : layoutHeaderConfig.show;
    const popout = (_g = (_f = (_e = this._headerConfig) === null || _e === void 0 ? void 0 : _e.popout) !== null && _f !== void 0 ? _f : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.popout) !== null && _g !== void 0 ? _g : layoutHeaderConfig.popout;
    const maximise = (_k = (_j = (_h = this._headerConfig) === null || _h === void 0 ? void 0 : _h.maximise) !== null && _j !== void 0 ? _j : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.maximise) !== null && _k !== void 0 ? _k : layoutHeaderConfig.maximise;
    const close = (_o = (_m = (_l = this._headerConfig) === null || _l === void 0 ? void 0 : _l.close) !== null && _m !== void 0 ? _m : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.close) !== null && _o !== void 0 ? _o : layoutHeaderConfig.close;
    const minimise = (_r = (_q = (_p = this._headerConfig) === null || _p === void 0 ? void 0 : _p.minimise) !== null && _q !== void 0 ? _q : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.minimise) !== null && _r !== void 0 ? _r : layoutHeaderConfig.minimise;
    const tabDropdown = (_u = (_t = (_s = this._headerConfig) === null || _s === void 0 ? void 0 : _s.tabDropdown) !== null && _t !== void 0 ? _t : componentHeaderConfig === null || componentHeaderConfig === void 0 ? void 0 : componentHeaderConfig.tabDropdown) !== null && _u !== void 0 ? _u : layoutHeaderConfig.tabDropdown;
    this._maximisedEnabled = maximise !== false;
    const headerSettings = {
      show: show !== false,
      side: show === false ? Side.top : show,
      popoutEnabled: popout !== false,
      popoutLabel: popout === false ? "" : popout,
      maximiseEnabled: this._maximisedEnabled,
      maximiseLabel: maximise === false ? "" : maximise,
      closeEnabled: close !== false,
      closeLabel: close === false ? "" : close,
      minimiseEnabled: true,
      minimiseLabel: minimise,
      tabDropdownEnabled: tabDropdown !== false,
      tabDropdownLabel: tabDropdown === false ? "" : tabDropdown
    };
    this._header = new Header(layoutManager, this, headerSettings, config.isClosable && close !== false, () => this.getActiveComponentItem(), () => this.remove(), () => this.handlePopoutEvent(), () => this.toggleMaximise(), (ev) => this.handleHeaderClickEvent(ev), (ev) => this.handleHeaderTouchStartEvent(ev), (item) => this.handleHeaderComponentRemoveEvent(item), (item) => this.handleHeaderComponentFocusEvent(item), (x, y, dragListener, item) => this.handleHeaderComponentStartDragEvent(x, y, dragListener, item));
    this.isStack = true;
    this._childElementContainer = document.createElement("section");
    this._childElementContainer.classList.add("lm_items");
    this.on("resize", this._resizeListener);
    if (this._maximisedEnabled) {
      this.on("maximised", this._maximisedListener);
      this.on("minimised", this._minimisedListener);
    }
    this.element.appendChild(this._header.element);
    this.element.appendChild(this._childElementContainer);
    this.setupHeaderPosition();
    this._header.updateClosability();
  }
  get childElementContainer() {
    return this._childElementContainer;
  }
  get header() {
    return this._header;
  }
  get headerShow() {
    return this._header.show;
  }
  get headerSide() {
    return this._header.side;
  }
  get headerLeftRightSided() {
    return this._header.leftRightSided;
  }
  get contentAreaDimensions() {
    return this._contentAreaDimensions;
  }
  get initialWantMaximise() {
    return this._initialWantMaximise;
  }
  get isMaximised() {
    return this === this.layoutManager.maximisedStack;
  }
  get stackParent() {
    if (!this.parent) {
      throw new Error("Stack should always have a parent");
    }
    return this.parent;
  }
  updateSize() {
    this.layoutManager.beginVirtualSizedContainerAdding();
    try {
      this.updateNodeSize();
      this.updateContentItemsSize();
    } finally {
      this.layoutManager.endVirtualSizedContainerAdding();
    }
  }
  init() {
    if (this.isInitialised === true)
      return;
    this.updateNodeSize();
    for (let i = 0; i < this.contentItems.length; i++) {
      this._childElementContainer.appendChild(this.contentItems[i].element);
    }
    super.init();
    const contentItems = this.contentItems;
    const contentItemCount = contentItems.length;
    if (contentItemCount > 0) {
      if (this._initialActiveItemIndex < 0 || this._initialActiveItemIndex >= contentItemCount) {
        throw new Error(`ActiveItemIndex out of range: ${this._initialActiveItemIndex} id: ${this.id}`);
      } else {
        for (let i = 0; i < contentItemCount; i++) {
          const contentItem = contentItems[i];
          if (!(contentItem instanceof ComponentItem)) {
            throw new Error(`Stack Content Item is not of type ComponentItem: ${i} id: ${this.id}`);
          } else {
            this._header.createTab(contentItem, i);
            contentItem.hide();
          }
        }
        this.setActiveComponentItem(contentItems[this._initialActiveItemIndex], false);
        this._header.updateTabSizes();
      }
    }
    this._header.updateClosability();
    this.initContentItems();
  }
  setActiveContentItem(item) {
    if (!ContentItem.isComponentItem(item)) {
      throw new Error("Stack.setActiveContentItem: item is not a ComponentItem");
    } else {
      this.setActiveComponentItem(item, false);
    }
  }
  setActiveComponentItem(componentItem, focus, suppressFocusEvent = false) {
    if (this._activeComponentItem !== componentItem) {
      if (this.contentItems.indexOf(componentItem) === -1) {
        throw new Error("componentItem is not a child of this stack");
      } else {
        if (this._activeComponentItem !== void 0) {
          this._activeComponentItem.hide();
        }
        this._activeComponentItem = componentItem;
        this._header.processActiveComponentChanged(componentItem);
        componentItem.show();
        this.emit("activeContentItemChanged", componentItem);
        this.layoutManager.emit("activeContentItemChanged", componentItem);
        this.emitStateChangedEvent();
      }
    }
    if (this.focused || focus) {
      this.layoutManager.setFocusedComponentItem(componentItem, suppressFocusEvent);
    }
  }
  getActiveContentItem() {
    var _a;
    return (_a = this.getActiveComponentItem()) !== null && _a !== void 0 ? _a : null;
  }
  getActiveComponentItem() {
    return this._activeComponentItem;
  }
  focusActiveContentItem() {
    var _a;
    (_a = this._activeComponentItem) === null || _a === void 0 ? void 0 : _a.focus();
  }
  setFocusedValue(value) {
    this._header.applyFocusedValue(value);
    super.setFocusedValue(value);
  }
  setRowColumnClosable(value) {
    this._header.setRowColumnClosable(value);
  }
  newComponent(componentType, componentState, title, index) {
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    return this.newItem(itemConfig, index);
  }
  addComponent(componentType, componentState, title, index) {
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    return this.addItem(itemConfig, index);
  }
  newItem(itemConfig, index) {
    index = this.addItem(itemConfig, index);
    return this.contentItems[index];
  }
  addItem(itemConfig, index) {
    this.layoutManager.checkMinimiseMaximisedStack();
    const resolvedItemConfig = ItemConfig.resolve(itemConfig);
    const contentItem = this.layoutManager.createAndInitContentItem(resolvedItemConfig, this);
    return this.addChild(contentItem, index);
  }
  addChild(contentItem, index, focus = false) {
    if (index !== void 0 && index > this.contentItems.length) {
      index -= 1;
      throw new AssertError("SAC99728");
    }
    if (!(contentItem instanceof ComponentItem)) {
      throw new AssertError("SACC88532");
    } else {
      index = super.addChild(contentItem, index);
      this._childElementContainer.appendChild(contentItem.element);
      this._header.createTab(contentItem, index);
      this.setActiveComponentItem(contentItem, focus);
      this._header.updateTabSizes();
      this.updateSize();
      this._header.updateClosability();
      this.emitStateChangedEvent();
      return index;
    }
  }
  removeChild(contentItem, keepChild) {
    const componentItem = contentItem;
    const index = this.contentItems.indexOf(componentItem);
    const stackWillBeDeleted = this.contentItems.length === 1;
    if (this._activeComponentItem === componentItem) {
      if (componentItem.focused) {
        componentItem.blur();
      }
      if (!stackWillBeDeleted) {
        const newActiveComponentIdx = index === 0 ? 1 : index - 1;
        this.setActiveComponentItem(this.contentItems[newActiveComponentIdx], false);
      }
    }
    this._header.removeTab(componentItem);
    super.removeChild(componentItem, keepChild);
    if (!stackWillBeDeleted) {
      this._header.updateClosability();
    }
    this.emitStateChangedEvent();
  }
  toggleMaximise() {
    if (this.isMaximised) {
      this.minimise();
    } else {
      this.maximise();
    }
  }
  maximise() {
    if (!this.isMaximised) {
      this.layoutManager.setMaximisedStack(this);
      const contentItems = this.contentItems;
      const contentItemCount = contentItems.length;
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        if (contentItem instanceof ComponentItem) {
          contentItem.enterStackMaximised();
        } else {
          throw new AssertError("SMAXI87773");
        }
      }
      this.emitStateChangedEvent();
    }
  }
  minimise() {
    if (this.isMaximised) {
      this.layoutManager.setMaximisedStack(void 0);
      const contentItems = this.contentItems;
      const contentItemCount = contentItems.length;
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        if (contentItem instanceof ComponentItem) {
          contentItem.exitStackMaximised();
        } else {
          throw new AssertError("SMINI87773");
        }
      }
      this.emitStateChangedEvent();
    }
  }
  destroy() {
    var _a;
    if ((_a = this._activeComponentItem) === null || _a === void 0 ? void 0 : _a.focused) {
      this._activeComponentItem.blur();
    }
    super.destroy();
    this.off("resize", this._resizeListener);
    if (this._maximisedEnabled) {
      this.off("maximised", this._maximisedListener);
      this.off("minimised", this._minimisedListener);
    }
    this._header.destroy();
  }
  toConfig() {
    let activeItemIndex;
    if (this._activeComponentItem) {
      activeItemIndex = this.contentItems.indexOf(this._activeComponentItem);
      if (activeItemIndex < 0) {
        throw new Error("active component item not found in stack");
      }
    }
    if (this.contentItems.length > 0 && activeItemIndex === void 0) {
      throw new Error("expected non-empty stack to have an active component item");
    } else {
      const result = {
        type: "stack",
        content: this.calculateConfigContent(),
        width: this.width,
        minWidth: this.minWidth,
        height: this.height,
        minHeight: this.minHeight,
        id: this.id,
        isClosable: this.isClosable,
        maximised: this.isMaximised,
        header: this.createHeaderConfig(),
        activeItemIndex
      };
      return result;
    }
  }
  onDrop(contentItem, area) {
    if (this._dropSegment === "header") {
      this.resetHeaderDropZone();
      if (this._dropIndex === void 0) {
        throw new UnexpectedUndefinedError("SODDI68990");
      } else {
        this.addChild(contentItem, this._dropIndex);
        return;
      }
    }
    if (this._dropSegment === "body") {
      this.addChild(contentItem, 0, true);
      return;
    }
    const isVertical = this._dropSegment === "top" || this._dropSegment === "bottom";
    const isHorizontal = this._dropSegment === "left" || this._dropSegment === "right";
    const insertBefore = this._dropSegment === "top" || this._dropSegment === "left";
    const hasCorrectParent = isVertical && this.stackParent.isColumn || isHorizontal && this.stackParent.isRow;
    const dimension = isVertical ? "height" : "width";
    if (contentItem.isComponent) {
      const itemConfig = ResolvedStackItemConfig.createDefault();
      itemConfig.header = this.createHeaderConfig();
      const stack = this.layoutManager.createAndInitContentItem(itemConfig, this);
      stack.addChild(contentItem);
      contentItem = stack;
    }
    if (contentItem.type === ItemType.row || contentItem.type === ItemType.column) {
      const itemConfig = ResolvedStackItemConfig.createDefault();
      itemConfig.header = this.createHeaderConfig();
      const stack = this.layoutManager.createContentItem(itemConfig, this);
      stack.addChild(contentItem);
      contentItem = stack;
    }
    if (hasCorrectParent) {
      const index = this.stackParent.contentItems.indexOf(this);
      this.stackParent.addChild(contentItem, insertBefore ? index : index + 1, true);
      this[dimension] *= 0.5;
      contentItem[dimension] = this[dimension];
      this.stackParent.updateSize();
    } else {
      const type = isVertical ? ItemType.column : ItemType.row;
      const itemConfig = ResolvedItemConfig.createDefault(type);
      const rowOrColumn = this.layoutManager.createContentItem(itemConfig, this);
      this.stackParent.replaceChild(this, rowOrColumn);
      rowOrColumn.addChild(contentItem, insertBefore ? 0 : void 0, true);
      rowOrColumn.addChild(this, insertBefore ? void 0 : 0, true);
      this[dimension] = 50;
      contentItem[dimension] = 50;
      rowOrColumn.updateSize();
    }
  }
  highlightDropZone(x, y) {
    for (const key in this._contentAreaDimensions) {
      const segment = key;
      const area = this._contentAreaDimensions[segment].hoverArea;
      if (area.x1 < x && area.x2 > x && area.y1 < y && area.y2 > y) {
        if (segment === "header") {
          this._dropSegment = "header";
          this.highlightHeaderDropZone(this._header.leftRightSided ? y : x);
        } else {
          this.resetHeaderDropZone();
          this.highlightBodyDropZone(segment);
        }
        return;
      }
    }
  }
  getArea() {
    if (this.element.style.display === "none") {
      return null;
    }
    const headerArea = super.getElementArea(this._header.element);
    const contentArea = super.getElementArea(this._childElementContainer);
    if (headerArea === null || contentArea === null) {
      throw new UnexpectedNullError("SGAHC13086");
    }
    const contentWidth = contentArea.x2 - contentArea.x1;
    const contentHeight = contentArea.y2 - contentArea.y1;
    this._contentAreaDimensions = {
      header: {
        hoverArea: {
          x1: headerArea.x1,
          y1: headerArea.y1,
          x2: headerArea.x2,
          y2: headerArea.y2
        },
        highlightArea: {
          x1: headerArea.x1,
          y1: headerArea.y1,
          x2: headerArea.x2,
          y2: headerArea.y2
        }
      }
    };
    if (this.contentItems.length === 0) {
      this._contentAreaDimensions.body = {
        hoverArea: {
          x1: contentArea.x1,
          y1: contentArea.y1,
          x2: contentArea.x2,
          y2: contentArea.y2
        },
        highlightArea: {
          x1: contentArea.x1,
          y1: contentArea.y1,
          x2: contentArea.x2,
          y2: contentArea.y2
        }
      };
      return super.getElementArea(this.element);
    } else {
      this._contentAreaDimensions.left = {
        hoverArea: {
          x1: contentArea.x1,
          y1: contentArea.y1,
          x2: contentArea.x1 + contentWidth * 0.25,
          y2: contentArea.y2
        },
        highlightArea: {
          x1: contentArea.x1,
          y1: contentArea.y1,
          x2: contentArea.x1 + contentWidth * 0.5,
          y2: contentArea.y2
        }
      };
      this._contentAreaDimensions.top = {
        hoverArea: {
          x1: contentArea.x1 + contentWidth * 0.25,
          y1: contentArea.y1,
          x2: contentArea.x1 + contentWidth * 0.75,
          y2: contentArea.y1 + contentHeight * 0.5
        },
        highlightArea: {
          x1: contentArea.x1,
          y1: contentArea.y1,
          x2: contentArea.x2,
          y2: contentArea.y1 + contentHeight * 0.5
        }
      };
      this._contentAreaDimensions.right = {
        hoverArea: {
          x1: contentArea.x1 + contentWidth * 0.75,
          y1: contentArea.y1,
          x2: contentArea.x2,
          y2: contentArea.y2
        },
        highlightArea: {
          x1: contentArea.x1 + contentWidth * 0.5,
          y1: contentArea.y1,
          x2: contentArea.x2,
          y2: contentArea.y2
        }
      };
      this._contentAreaDimensions.bottom = {
        hoverArea: {
          x1: contentArea.x1 + contentWidth * 0.25,
          y1: contentArea.y1 + contentHeight * 0.5,
          x2: contentArea.x1 + contentWidth * 0.75,
          y2: contentArea.y2
        },
        highlightArea: {
          x1: contentArea.x1,
          y1: contentArea.y1 + contentHeight * 0.5,
          x2: contentArea.x2,
          y2: contentArea.y2
        }
      };
      return super.getElementArea(this.element);
    }
  }
  positionHeader(position) {
    if (this._header.side !== position) {
      this._header.setSide(position);
      this._headerSideChanged = true;
      this.setupHeaderPosition();
    }
  }
  updateNodeSize() {
    if (this.element.style.display !== "none") {
      const content = getElementWidthAndHeight(this.element);
      if (this._header.show) {
        const dimension = this._header.leftRightSided ? WidthOrHeightPropertyName.width : WidthOrHeightPropertyName.height;
        content[dimension] -= this.layoutManager.layoutConfig.dimensions.headerHeight;
      }
      this._childElementContainer.style.width = numberToPixels(content.width);
      this._childElementContainer.style.height = numberToPixels(content.height);
      for (let i = 0; i < this.contentItems.length; i++) {
        this.contentItems[i].element.style.width = numberToPixels(content.width);
        this.contentItems[i].element.style.height = numberToPixels(content.height);
      }
      this.emit("resize");
      this.emitStateChangedEvent();
    }
  }
  highlightHeaderDropZone(x) {
    const tabsLength = this._header.lastVisibleTabIndex + 1;
    const dropTargetIndicator = this.layoutManager.dropTargetIndicator;
    if (dropTargetIndicator === null) {
      throw new UnexpectedNullError("SHHDZDTI97110");
    }
    let area;
    if (tabsLength === 0) {
      const headerOffset = getJQueryOffset(this._header.element);
      const elementHeight = getElementHeight(this._header.element);
      area = {
        x1: headerOffset.left,
        x2: headerOffset.left + 100,
        y1: headerOffset.top + elementHeight - 20,
        y2: headerOffset.top + elementHeight
      };
    } else {
      let tabIndex = 0;
      let isAboveTab = false;
      let tabTop;
      let tabLeft;
      let tabWidth;
      let tabElement;
      do {
        tabElement = this._header.tabs[tabIndex].element;
        const offset = getJQueryOffset(tabElement);
        if (this._header.leftRightSided) {
          tabLeft = offset.top;
          tabTop = offset.left;
          tabWidth = getElementHeight(tabElement);
        } else {
          tabLeft = offset.left;
          tabTop = offset.top;
          tabWidth = getElementWidth(tabElement);
        }
        if (x >= tabLeft && x < tabLeft + tabWidth) {
          isAboveTab = true;
        } else {
          tabIndex++;
        }
      } while (tabIndex < tabsLength && !isAboveTab);
      if (isAboveTab === false && x < tabLeft) {
        return;
      }
      const halfX = tabLeft + tabWidth / 2;
      if (x < halfX) {
        this._dropIndex = tabIndex;
        tabElement.insertAdjacentElement("beforebegin", this.layoutManager.tabDropPlaceholder);
      } else {
        this._dropIndex = Math.min(tabIndex + 1, tabsLength);
        tabElement.insertAdjacentElement("afterend", this.layoutManager.tabDropPlaceholder);
      }
      const tabDropPlaceholderOffset = getJQueryOffset(this.layoutManager.tabDropPlaceholder);
      const tabDropPlaceholderWidth = getElementWidth(this.layoutManager.tabDropPlaceholder);
      if (this._header.leftRightSided) {
        const placeHolderTop = tabDropPlaceholderOffset.top;
        area = {
          x1: tabTop,
          x2: tabTop + tabElement.clientHeight,
          y1: placeHolderTop,
          y2: placeHolderTop + tabDropPlaceholderWidth
        };
      } else {
        const placeHolderLeft = tabDropPlaceholderOffset.left;
        area = {
          x1: placeHolderLeft,
          x2: placeHolderLeft + tabDropPlaceholderWidth,
          y1: tabTop,
          y2: tabTop + tabElement.clientHeight
        };
      }
    }
    dropTargetIndicator.highlightArea(area);
    return;
  }
  resetHeaderDropZone() {
    this.layoutManager.tabDropPlaceholder.remove();
  }
  setupHeaderPosition() {
    setElementDisplayVisibility(this._header.element, this._header.show);
    this.element.classList.remove("lm_left", "lm_right", "lm_bottom");
    if (this._header.leftRightSided) {
      this.element.classList.add("lm_" + this._header.side);
    }
    this.updateSize();
  }
  highlightBodyDropZone(segment) {
    if (this._contentAreaDimensions === void 0) {
      throw new UnexpectedUndefinedError("SHBDZC82265");
    } else {
      const highlightArea = this._contentAreaDimensions[segment].highlightArea;
      const dropTargetIndicator = this.layoutManager.dropTargetIndicator;
      if (dropTargetIndicator === null) {
        throw new UnexpectedNullError("SHBDZD96110");
      } else {
        dropTargetIndicator.highlightArea(highlightArea);
        this._dropSegment = segment;
      }
    }
  }
  handleResize() {
    this._header.updateTabSizes();
  }
  handleMaximised() {
    this._header.processMaximised();
  }
  handleMinimised() {
    this._header.processMinimised();
  }
  handlePopoutEvent() {
    this.popout();
  }
  handleHeaderClickEvent(ev) {
    const eventName = EventEmitter.headerClickEventName;
    const bubblingEvent = new EventEmitter.ClickBubblingEvent(eventName, this, ev);
    this.emit(eventName, bubblingEvent);
  }
  handleHeaderTouchStartEvent(ev) {
    const eventName = EventEmitter.headerTouchStartEventName;
    const bubblingEvent = new EventEmitter.TouchStartBubblingEvent(eventName, this, ev);
    this.emit(eventName, bubblingEvent);
  }
  handleHeaderComponentRemoveEvent(item) {
    this.removeChild(item, false);
  }
  handleHeaderComponentFocusEvent(item) {
    this.setActiveComponentItem(item, true);
  }
  handleHeaderComponentStartDragEvent(x, y, dragListener, componentItem) {
    if (this.isMaximised === true) {
      this.toggleMaximise();
    }
    this.layoutManager.startComponentDrag(x, y, dragListener, componentItem, this);
  }
  createHeaderConfig() {
    if (!this._headerSideChanged) {
      return ResolvedHeaderedItemConfig.Header.createCopy(this._headerConfig);
    } else {
      const show = this._header.show ? this._header.side : false;
      let result = ResolvedHeaderedItemConfig.Header.createCopy(this._headerConfig, show);
      if (result === void 0) {
        result = {
          show,
          popout: void 0,
          maximise: void 0,
          close: void 0,
          minimise: void 0,
          tabDropdown: void 0
        };
      }
      return result;
    }
  }
  emitStateChangedEvent() {
    this.emitBaseBubblingEvent("stateChanged");
  }
}
(function(Stack2) {
  function createElement(document2) {
    const element = document2.createElement("div");
    element.classList.add("lm_item");
    element.classList.add("lm_stack");
    return element;
  }
  Stack2.createElement = createElement;
})(Stack || (Stack = {}));
class DragProxy extends EventEmitter {
  constructor(x, y, _dragListener, _layoutManager, _componentItem, _originalParent) {
    super();
    this._dragListener = _dragListener;
    this._layoutManager = _layoutManager;
    this._componentItem = _componentItem;
    this._originalParent = _originalParent;
    this._area = null;
    this._lastValidArea = null;
    this._dragListener.on("drag", (offsetX, offsetY, event) => this.onDrag(offsetX, offsetY, event));
    this._dragListener.on("dragStop", () => this.onDrop());
    this.createDragProxyElements(x, y);
    if (this._componentItem.parent === null) {
      throw new UnexpectedNullError("DPC10097");
    }
    this._componentItemFocused = this._componentItem.focused;
    if (this._componentItemFocused) {
      this._componentItem.blur();
    }
    this._componentItem.parent.removeChild(this._componentItem, true);
    this.setDimensions();
    document.body.appendChild(this._element);
    this.determineMinMaxXY();
    if (this._layoutManager.layoutConfig.settings.constrainDragToContainer) {
      const constrainedPosition = this.getXYWithinMinMax(x, y);
      x = constrainedPosition.x;
      y = constrainedPosition.y;
    }
    this._layoutManager.calculateItemAreas();
    this.setDropPosition(x, y);
  }
  get element() {
    return this._element;
  }
  createDragProxyElements(initialX, initialY) {
    this._element = document.createElement("div");
    this._element.classList.add("lm_dragProxy");
    const headerElement = document.createElement("div");
    headerElement.classList.add("lm_header");
    const tabsElement = document.createElement("div");
    tabsElement.classList.add("lm_tabs");
    const tabElement = document.createElement("div");
    tabElement.classList.add("lm_tab");
    const titleElement = document.createElement("span");
    titleElement.classList.add("lm_title");
    tabElement.appendChild(titleElement);
    tabsElement.appendChild(tabElement);
    headerElement.appendChild(tabsElement);
    this._proxyContainerElement = document.createElement("div");
    this._proxyContainerElement.classList.add("lm_content");
    this._element.appendChild(headerElement);
    this._element.appendChild(this._proxyContainerElement);
    if (this._originalParent instanceof Stack && this._originalParent.headerShow) {
      this._sided = this._originalParent.headerLeftRightSided;
      this._element.classList.add("lm_" + this._originalParent.headerSide);
      if ([Side.right, Side.bottom].indexOf(this._originalParent.headerSide) >= 0) {
        this._proxyContainerElement.insertAdjacentElement("afterend", headerElement);
      }
    }
    this._element.style.left = numberToPixels(initialX);
    this._element.style.top = numberToPixels(initialY);
    tabElement.setAttribute("title", this._componentItem.title);
    titleElement.insertAdjacentText("afterbegin", this._componentItem.title);
    this._proxyContainerElement.appendChild(this._componentItem.element);
  }
  determineMinMaxXY() {
    const offset = getJQueryOffset(this._layoutManager.container);
    this._minX = offset.left;
    this._minY = offset.top;
    const {width: containerWidth, height: containerHeight} = getElementWidthAndHeight(this._layoutManager.container);
    this._maxX = containerWidth + this._minX;
    this._maxY = containerHeight + this._minY;
  }
  getXYWithinMinMax(x, y) {
    if (x <= this._minX) {
      x = Math.ceil(this._minX + 1);
    } else if (x >= this._maxX) {
      x = Math.floor(this._maxX - 1);
    }
    if (y <= this._minY) {
      y = Math.ceil(this._minY + 1);
    } else if (y >= this._maxY) {
      y = Math.floor(this._maxY - 1);
    }
    return {x, y};
  }
  onDrag(offsetX, offsetY, event) {
    const x = event.pageX;
    const y = event.pageY;
    if (!this._layoutManager.layoutConfig.settings.constrainDragToContainer) {
      this.setDropPosition(x, y);
    } else {
      const isWithinContainer = x > this._minX && x < this._maxX && y > this._minY && y < this._maxY;
      if (isWithinContainer) {
        this.setDropPosition(x, y);
      }
    }
    this._componentItem.drag();
  }
  setDropPosition(x, y) {
    this._element.style.left = numberToPixels(x);
    this._element.style.top = numberToPixels(y);
    this._area = this._layoutManager.getArea(x, y);
    if (this._area !== null) {
      this._lastValidArea = this._area;
      this._area.contentItem.highlightDropZone(x, y, this._area);
    }
  }
  onDrop() {
    const dropTargetIndicator = this._layoutManager.dropTargetIndicator;
    if (dropTargetIndicator === null) {
      throw new UnexpectedNullError("DPOD30011");
    } else {
      dropTargetIndicator.hide();
    }
    this._componentItem.exitDragMode();
    let droppedComponentItem;
    if (this._area !== null) {
      droppedComponentItem = this._componentItem;
      this._area.contentItem.onDrop(droppedComponentItem, this._area);
    } else if (this._lastValidArea !== null) {
      droppedComponentItem = this._componentItem;
      const newParentContentItem = this._lastValidArea.contentItem;
      newParentContentItem.onDrop(droppedComponentItem, this._lastValidArea);
    } else if (this._originalParent) {
      droppedComponentItem = this._componentItem;
      this._originalParent.addChild(droppedComponentItem);
    } else {
      this._componentItem.destroy();
    }
    this._element.remove();
    this._layoutManager.emit("itemDropped", this._componentItem);
    if (this._componentItemFocused && droppedComponentItem !== void 0) {
      droppedComponentItem.focus();
    }
  }
  setDimensions() {
    const dimensions = this._layoutManager.layoutConfig.dimensions;
    if (dimensions === void 0) {
      throw new Error("DragProxy.setDimensions: dimensions undefined");
    }
    let width = dimensions.dragProxyWidth;
    let height = dimensions.dragProxyHeight;
    if (width === void 0 || height === void 0) {
      throw new Error("DragProxy.setDimensions: width and/or height undefined");
    }
    const headerHeight = this._layoutManager.layoutConfig.header.show === false ? 0 : dimensions.headerHeight;
    this._element.style.width = numberToPixels(width);
    this._element.style.height = numberToPixels(height);
    width -= this._sided ? headerHeight : 0;
    height -= !this._sided ? headerHeight : 0;
    this._proxyContainerElement.style.width = numberToPixels(width);
    this._proxyContainerElement.style.height = numberToPixels(height);
    this._componentItem.enterDragMode(width, height);
    this._componentItem.show();
  }
}
class DragSource {
  constructor(_layoutManager, _element, _extraAllowableChildTargets, _componentTypeOrFtn, _componentState, _title) {
    this._layoutManager = _layoutManager;
    this._element = _element;
    this._extraAllowableChildTargets = _extraAllowableChildTargets;
    this._componentTypeOrFtn = _componentTypeOrFtn;
    this._componentState = _componentState;
    this._title = _title;
    this._dragListener = null;
    this._dummyGroundContainer = document.createElement("div");
    const dummyRootItemConfig = ResolvedRowOrColumnItemConfig.createDefault("row");
    this._dummyGroundContentItem = new GroundItem(this._layoutManager, dummyRootItemConfig, this._dummyGroundContainer);
    this.createDragListener();
  }
  destroy() {
    this.removeDragListener();
  }
  createDragListener() {
    this.removeDragListener();
    this._dragListener = new DragListener(this._element, this._extraAllowableChildTargets);
    this._dragListener.on("dragStart", (x, y) => this.onDragStart(x, y));
    this._dragListener.on("dragStop", () => this.onDragStop());
  }
  onDragStart(x, y) {
    let componentType;
    let componentState;
    let title;
    if (typeof this._componentTypeOrFtn === "function") {
      const dragSourceItemConfig = this._componentTypeOrFtn();
      componentType = dragSourceItemConfig.type;
      componentState = dragSourceItemConfig.state;
      title = dragSourceItemConfig.title;
    } else {
      componentType = this._componentTypeOrFtn;
      componentState = this._componentState;
      title = this._title;
    }
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    const resolvedItemConfig = ComponentItemConfig.resolve(itemConfig);
    const componentItem = new ComponentItem(this._layoutManager, resolvedItemConfig, this._dummyGroundContentItem);
    this._dummyGroundContentItem.contentItems.push(componentItem);
    if (this._dragListener === null) {
      throw new UnexpectedNullError("DSODSD66746");
    } else {
      const dragProxy = new DragProxy(x, y, this._dragListener, this._layoutManager, componentItem, this._dummyGroundContentItem);
      const transitionIndicator = this._layoutManager.transitionIndicator;
      if (transitionIndicator === null) {
        throw new UnexpectedNullError("DSODST66746");
      } else {
        transitionIndicator.transitionElements(this._element, dragProxy.element);
      }
    }
  }
  onDragStop() {
    this.createDragListener();
  }
  removeDragListener() {
    if (this._dragListener !== null) {
      this._dragListener.destroy();
      this._dragListener = null;
    }
  }
}
var I18nStrings;
(function(I18nStrings2) {
  let initialised = false;
  const infosObject = {
    PopoutCannotBeCreatedWithGroundItemConfig: {
      id: 0,
      default: "Popout cannot be created with ground ItemConfig"
    },
    PleaseRegisterAConstructorFunction: {
      id: 1,
      default: "Please register a constructor function"
    },
    ComponentTypeNotRegisteredAndBindComponentEventHandlerNotAssigned: {
      id: 2,
      default: "Component type not registered and BindComponentEvent handler not assigned"
    },
    ComponentIsAlreadyRegistered: {
      id: 3,
      default: "Component is already registered"
    },
    ComponentIsNotVirtuable: {
      id: 4,
      default: "Component is not virtuable. Requires rootHtmlElement field/getter"
    },
    VirtualComponentDoesNotHaveRootHtmlElement: {
      id: 5,
      default: 'Virtual component does not have getter "rootHtmlElement"'
    },
    ItemConfigIsNotTypeComponent: {
      id: 6,
      default: "ItemConfig is not of type component"
    }
  };
  I18nStrings2.idCount = Object.keys(infosObject).length;
  const infos = Object.values(infosObject);
  function checkInitialise() {
    if (!initialised) {
      for (let i = 0; i < I18nStrings2.idCount; i++) {
        const info = infos[i];
        if (info.id !== i) {
          throw new AssertError("INSI00110", `${i}: ${info.id}`);
        } else {
          i18nStrings[i] = info.default;
        }
      }
    }
    initialised = true;
  }
  I18nStrings2.checkInitialise = checkInitialise;
})(I18nStrings || (I18nStrings = {}));
const i18nStrings = new Array(I18nStrings.idCount);
class DropTargetIndicator {
  constructor() {
    this._element = document.createElement("div");
    this._element.classList.add("lm_dropTargetIndicator");
    const innerElement = document.createElement("div");
    innerElement.classList.add("lm_inner");
    this._element.appendChild(innerElement);
    document.body.appendChild(this._element);
  }
  destroy() {
    this._element.remove();
  }
  highlightArea(area) {
    this._element.style.left = numberToPixels(area.x1);
    this._element.style.top = numberToPixels(area.y1);
    this._element.style.width = numberToPixels(area.x2 - area.x1);
    this._element.style.height = numberToPixels(area.y2 - area.y1);
    this._element.style.display = "block";
  }
  hide() {
    setElementDisplayVisibility(this._element, false);
  }
}
class TransitionIndicator {
  constructor() {
    this._element = document.createElement("div");
    this._element.classList.add("lm_transition_indicator");
    document.body.appendChild(this._element);
    this._toElement = null;
    this._fromDimensions = null;
    this._totalAnimationDuration = 200;
    this._animationStartTime = null;
  }
  destroy() {
    this._element.remove();
  }
  transitionElements(fromElement, toElement) {
    return;
  }
  nextAnimationFrame() {
  }
  measure(element) {
    const rect = element.getBoundingClientRect();
    return {
      left: rect.left,
      top: rect.top,
      width: element.offsetWidth,
      height: element.offsetHeight
    };
  }
}
class EventHub extends EventEmitter {
  constructor(_layoutManager) {
    super();
    this._layoutManager = _layoutManager;
    this._childEventListener = (childEvent) => this.onEventFromChild(childEvent);
    globalThis.addEventListener(EventHub.ChildEventName, this._childEventListener, {passive: true});
  }
  emit(eventName, ...args) {
    if (eventName === "userBroadcast") {
      this.emitUserBroadcast(...args);
    } else {
      super.emit(eventName, ...args);
    }
  }
  emitUserBroadcast(...args) {
    this.handleUserBroadcastEvent("userBroadcast", args);
  }
  destroy() {
    globalThis.removeEventListener(EventHub.ChildEventName, this._childEventListener);
  }
  handleUserBroadcastEvent(eventName, args) {
    if (this._layoutManager.isSubWindow) {
      this.propagateToParent(eventName, args);
    } else {
      this.propagateToThisAndSubtree(eventName, args);
    }
  }
  onEventFromChild(event) {
    const detail = event.detail;
    this.handleUserBroadcastEvent(detail.eventName, detail.args);
  }
  propagateToParent(eventName, args) {
    const detail = {
      layoutManager: this._layoutManager,
      eventName,
      args
    };
    const eventInit = {
      bubbles: true,
      cancelable: true,
      detail
    };
    const event = new CustomEvent(EventHub.ChildEventName, eventInit);
    const opener = globalThis.opener;
    if (opener === null) {
      throw new UnexpectedNullError("EHPTP15778");
    }
    opener.dispatchEvent(event);
  }
  propagateToThisAndSubtree(eventName, args) {
    this.emitUnknown(eventName, ...args);
    for (let i = 0; i < this._layoutManager.openPopouts.length; i++) {
      const childGl = this._layoutManager.openPopouts[i].getGlInstance();
      if (childGl) {
        childGl.eventHub.propagateToThisAndSubtree(eventName, args);
      }
    }
  }
}
(function(EventHub2) {
  EventHub2.ChildEventName = "gl_child_event";
})(EventHub || (EventHub = {}));
class LayoutManager extends EventEmitter {
  constructor(parameters) {
    super();
    this._isFullPage = false;
    this._isInitialised = false;
    this._groundItem = void 0;
    this._openPopouts = [];
    this._dropTargetIndicator = null;
    this._transitionIndicator = null;
    this._itemAreas = [];
    this._maximisePlaceholder = LayoutManager.createMaximisePlaceElement(document);
    this._tabDropPlaceholder = LayoutManager.createTabDropPlaceholderElement(document);
    this._dragSources = [];
    this._updatingColumnsResponsive = false;
    this._firstLoad = true;
    this._eventHub = new EventHub(this);
    this._width = null;
    this._height = null;
    this._virtualSizedContainers = [];
    this._virtualSizedContainerAddingBeginCount = 0;
    this._windowResizeListener = () => this.processResizeWithDebounce();
    this._windowUnloadListener = () => this.onUnload();
    this._maximisedStackBeforeDestroyedListener = (ev) => this.cleanupBeforeMaximisedStackDestroyed(ev);
    this.isSubWindow = parameters.isSubWindow;
    this._constructorOrSubWindowLayoutConfig = parameters.constructorOrSubWindowLayoutConfig;
    I18nStrings.checkInitialise();
    ConfigMinifier.checkInitialise();
    if (parameters.containerElement !== void 0) {
      this._containerElement = parameters.containerElement;
    }
  }
  get container() {
    return this._containerElement;
  }
  get isInitialised() {
    return this._isInitialised;
  }
  get groundItem() {
    return this._groundItem;
  }
  get root() {
    return this._groundItem;
  }
  get openPopouts() {
    return this._openPopouts;
  }
  get dropTargetIndicator() {
    return this._dropTargetIndicator;
  }
  get transitionIndicator() {
    return this._transitionIndicator;
  }
  get width() {
    return this._width;
  }
  get height() {
    return this._height;
  }
  get eventHub() {
    return this._eventHub;
  }
  get rootItem() {
    if (this._groundItem === void 0) {
      throw new Error("Cannot access rootItem before init");
    } else {
      const groundContentItems = this._groundItem.contentItems;
      if (groundContentItems.length === 0) {
        return void 0;
      } else {
        return this._groundItem.contentItems[0];
      }
    }
  }
  get focusedComponentItem() {
    return this._focusedComponentItem;
  }
  get tabDropPlaceholder() {
    return this._tabDropPlaceholder;
  }
  get maximisedStack() {
    return this._maximisedStack;
  }
  get deprecatedConstructor() {
    return !this.isSubWindow && this._constructorOrSubWindowLayoutConfig !== void 0;
  }
  destroy() {
    if (this._isInitialised) {
      if (this.layoutConfig.settings.closePopoutsOnUnload === true) {
        for (let i = 0; i < this._openPopouts.length; i++) {
          this._openPopouts[i].close();
        }
      }
      if (this._isFullPage) {
        globalThis.removeEventListener("resize", this._windowResizeListener);
      }
      globalThis.removeEventListener("unload", this._windowUnloadListener);
      globalThis.removeEventListener("beforeunload", this._windowUnloadListener);
      if (this._groundItem !== void 0) {
        this._groundItem.destroy();
      }
      this._tabDropPlaceholder.remove();
      if (this._dropTargetIndicator !== null) {
        this._dropTargetIndicator.destroy();
      }
      if (this._transitionIndicator !== null) {
        this._transitionIndicator.destroy();
      }
      this._eventHub.destroy();
      for (const dragSource of this._dragSources) {
        dragSource.destroy();
      }
      this._dragSources = [];
      this._isInitialised = false;
    }
  }
  minifyConfig(config) {
    return ResolvedLayoutConfig.minifyConfig(config);
  }
  unminifyConfig(config) {
    return ResolvedLayoutConfig.unminifyConfig(config);
  }
  init() {
    this.setContainer();
    this._dropTargetIndicator = new DropTargetIndicator();
    this._transitionIndicator = new TransitionIndicator();
    this.updateSizeFromContainer();
    let subWindowRootConfig;
    if (this.isSubWindow) {
      if (this._constructorOrSubWindowLayoutConfig === void 0) {
        throw new UnexpectedUndefinedError("LMIU07155");
      } else {
        if (ItemConfig.isComponent(this._constructorOrSubWindowLayoutConfig.root)) {
          subWindowRootConfig = this._constructorOrSubWindowLayoutConfig.root;
        } else {
          throw new AssertError("LMIC07155");
        }
        const resolvedLayoutConfig = LayoutConfig.resolve(this._constructorOrSubWindowLayoutConfig);
        this.layoutConfig = Object.assign(Object.assign({}, resolvedLayoutConfig), {root: void 0});
      }
    } else {
      if (this._constructorOrSubWindowLayoutConfig === void 0) {
        this.layoutConfig = ResolvedLayoutConfig.createDefault();
      } else {
        this.layoutConfig = LayoutConfig.resolve(this._constructorOrSubWindowLayoutConfig);
      }
    }
    const layoutConfig = this.layoutConfig;
    this._groundItem = new GroundItem(this, layoutConfig.root, this._containerElement);
    this._groundItem.init();
    this.checkLoadedLayoutMaximiseItem();
    this.bindEvents();
    this._isInitialised = true;
    this.adjustColumnsResponsive();
    this.emit("initialised");
    if (subWindowRootConfig !== void 0) {
      this.loadComponentAsRoot(subWindowRootConfig);
    }
  }
  loadLayout(layoutConfig) {
    if (!this.isInitialised) {
      throw new Error("GoldenLayout: Need to call init() if LayoutConfig with defined root passed to constructor");
    } else {
      if (this._groundItem === void 0) {
        throw new UnexpectedUndefinedError("LMLL11119");
      } else {
        this.createSubWindows();
        this.layoutConfig = LayoutConfig.resolve(layoutConfig);
        this._groundItem.loadRoot(this.layoutConfig.root);
        this.checkLoadedLayoutMaximiseItem();
        this.adjustColumnsResponsive();
      }
    }
  }
  saveLayout() {
    if (this._isInitialised === false) {
      throw new Error("Can't create config, layout not yet initialised");
    } else {
      if (this._groundItem === void 0) {
        throw new UnexpectedUndefinedError("LMTC18244");
      } else {
        const groundContent = this._groundItem.calculateConfigContent();
        let rootItemConfig;
        if (groundContent.length !== 1) {
          rootItemConfig = void 0;
        } else {
          rootItemConfig = groundContent[0];
        }
        this.reconcilePopoutWindows();
        const openPopouts = [];
        for (let i = 0; i < this._openPopouts.length; i++) {
          openPopouts.push(this._openPopouts[i].toConfig());
        }
        const config = {
          root: rootItemConfig,
          openPopouts,
          settings: ResolvedLayoutConfig.Settings.createCopy(this.layoutConfig.settings),
          dimensions: ResolvedLayoutConfig.Dimensions.createCopy(this.layoutConfig.dimensions),
          header: ResolvedLayoutConfig.Header.createCopy(this.layoutConfig.header),
          resolved: true
        };
        return config;
      }
    }
  }
  clear() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMCL11129");
    } else {
      this._groundItem.clearRoot();
    }
  }
  toConfig() {
    return this.saveLayout();
  }
  newComponent(componentType, componentState, title) {
    const componentItem = this.newComponentAtLocation(componentType, componentState, title);
    if (componentItem === void 0) {
      throw new AssertError("LMNC65588");
    } else {
      return componentItem;
    }
  }
  newComponentAtLocation(componentType, componentState, title, locationSelectors) {
    if (this._groundItem === void 0) {
      throw new Error("Cannot add component before init");
    } else {
      const location2 = this.addComponentAtLocation(componentType, componentState, title, locationSelectors);
      if (location2 === void 0) {
        return void 0;
      } else {
        const createdItem = location2.parentItem.contentItems[location2.index];
        if (!ContentItem.isComponentItem(createdItem)) {
          throw new AssertError("LMNC992877533");
        } else {
          return createdItem;
        }
      }
    }
  }
  addComponent(componentType, componentState, title) {
    const location2 = this.addComponentAtLocation(componentType, componentState, title);
    if (location2 === void 0) {
      throw new AssertError("LMAC99943");
    } else {
      return location2;
    }
  }
  addComponentAtLocation(componentType, componentState, title, locationSelectors) {
    const itemConfig = {
      type: "component",
      componentType,
      componentState,
      title
    };
    return this.addItemAtLocation(itemConfig, locationSelectors);
  }
  newItem(itemConfig) {
    const contentItem = this.newItemAtLocation(itemConfig);
    if (contentItem === void 0) {
      throw new AssertError("LMNC65588");
    } else {
      return contentItem;
    }
  }
  newItemAtLocation(itemConfig, locationSelectors) {
    if (this._groundItem === void 0) {
      throw new Error("Cannot add component before init");
    } else {
      const location2 = this.addItemAtLocation(itemConfig, locationSelectors);
      if (location2 === void 0) {
        return void 0;
      } else {
        const createdItem = location2.parentItem.contentItems[location2.index];
        return createdItem;
      }
    }
  }
  addItem(itemConfig) {
    const location2 = this.addItemAtLocation(itemConfig);
    if (location2 === void 0) {
      throw new AssertError("LMAI99943");
    } else {
      return location2;
    }
  }
  addItemAtLocation(itemConfig, locationSelectors) {
    if (this._groundItem === void 0) {
      throw new Error("Cannot add component before init");
    } else {
      if (locationSelectors === void 0) {
        locationSelectors = LayoutManager.defaultLocationSelectors;
      }
      const location2 = this.findFirstLocation(locationSelectors);
      if (location2 === void 0) {
        return void 0;
      } else {
        let parentItem = location2.parentItem;
        let addIdx;
        switch (parentItem.type) {
          case ItemType.ground: {
            const groundItem = parentItem;
            addIdx = groundItem.addItem(itemConfig, location2.index);
            if (addIdx >= 0) {
              parentItem = this._groundItem.contentItems[0];
            } else {
              addIdx = 0;
            }
            break;
          }
          case ItemType.row:
          case ItemType.column: {
            const rowOrColumn = parentItem;
            addIdx = rowOrColumn.addItem(itemConfig, location2.index);
            break;
          }
          case ItemType.stack: {
            if (!ItemConfig.isComponent(itemConfig)) {
              throw Error(i18nStrings[6]);
            } else {
              const stack = parentItem;
              addIdx = stack.addItem(itemConfig, location2.index);
              break;
            }
          }
          case ItemType.component: {
            throw new AssertError("LMAIALC87444602");
          }
          default:
            throw new UnreachableCaseError("LMAIALU98881733", parentItem.type);
        }
        if (ItemConfig.isComponent(itemConfig)) {
          const item = parentItem.contentItems[addIdx];
          if (ContentItem.isStack(item)) {
            parentItem = item;
            addIdx = 0;
          }
        }
        location2.parentItem = parentItem;
        location2.index = addIdx;
        return location2;
      }
    }
  }
  loadComponentAsRoot(itemConfig) {
    if (this._groundItem === void 0) {
      throw new Error("Cannot add item before init");
    } else {
      this._groundItem.loadComponentAsRoot(itemConfig);
    }
  }
  updateSize(width, height) {
    this.setSize(width, height);
  }
  setSize(width, height) {
    this._width = width;
    this._height = height;
    if (this._isInitialised === true) {
      if (this._groundItem === void 0) {
        throw new UnexpectedUndefinedError("LMUS18881");
      } else {
        this._groundItem.setSize(this._width, this._height);
        if (this._maximisedStack) {
          const {width: width2, height: height2} = getElementWidthAndHeight(this._containerElement);
          setElementWidth(this._maximisedStack.element, width2);
          setElementHeight(this._maximisedStack.element, height2);
          this._maximisedStack.updateSize();
        }
        this.adjustColumnsResponsive();
      }
    }
  }
  updateSizeFromContainer() {
    const {width, height} = getElementWidthAndHeight(this._containerElement);
    this.setSize(width, height);
  }
  updateRootSize() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMURS28881");
    } else {
      this._groundItem.updateSize();
    }
  }
  createAndInitContentItem(config, parent) {
    const newItem = this.createContentItem(config, parent);
    newItem.init();
    return newItem;
  }
  createContentItem(config, parent) {
    if (typeof config.type !== "string") {
      throw new ConfigurationError("Missing parameter 'type'", JSON.stringify(config));
    }
    if (ResolvedItemConfig.isComponentItem(config) && !(parent instanceof Stack) && !!parent && !(this.isSubWindow === true && parent instanceof GroundItem)) {
      const stackConfig = {
        type: ItemType.stack,
        content: [config],
        width: config.width,
        minWidth: config.minWidth,
        height: config.height,
        minHeight: config.minHeight,
        id: config.id,
        maximised: config.maximised,
        isClosable: config.isClosable,
        activeItemIndex: 0,
        header: void 0
      };
      config = stackConfig;
    }
    const contentItem = this.createContentItemFromConfig(config, parent);
    return contentItem;
  }
  findFirstComponentItemById(id) {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMFFCIBI82446");
    } else {
      return this.findFirstContentItemTypeByIdRecursive(ItemType.component, id, this._groundItem);
    }
  }
  createPopout(itemConfigOrContentItem, positionAndSize, parentId, indexInParent) {
    if (itemConfigOrContentItem instanceof ContentItem) {
      return this.createPopoutFromContentItem(itemConfigOrContentItem, positionAndSize, parentId, indexInParent);
    } else {
      return this.createPopoutFromItemConfig(itemConfigOrContentItem, positionAndSize, parentId, indexInParent);
    }
  }
  createPopoutFromContentItem(item, window2, parentId, indexInParent) {
    let parent = item.parent;
    let child = item;
    while (parent !== null && parent.contentItems.length === 1 && !parent.isGround) {
      child = parent;
      parent = parent.parent;
    }
    if (parent === null) {
      throw new UnexpectedNullError("LMCPFCI00834");
    } else {
      if (indexInParent === void 0) {
        indexInParent = parent.contentItems.indexOf(child);
      }
      if (parentId !== null) {
        parent.addPopInParentId(parentId);
      }
      if (window2 === void 0) {
        const windowLeft = globalThis.screenX || globalThis.screenLeft;
        const windowTop = globalThis.screenY || globalThis.screenTop;
        const offsetLeft = item.element.offsetLeft;
        const offsetTop = item.element.offsetTop;
        const {width, height} = getElementWidthAndHeight(item.element);
        window2 = {
          left: windowLeft + offsetLeft,
          top: windowTop + offsetTop,
          width,
          height
        };
      }
      const itemConfig = item.toConfig();
      item.remove();
      if (!ResolvedRootItemConfig.isRootItemConfig(itemConfig)) {
        throw new Error(`${i18nStrings[0]}`);
      } else {
        return this.createPopoutFromItemConfig(itemConfig, window2, parentId, indexInParent);
      }
    }
  }
  beginVirtualSizedContainerAdding() {
    if (++this._virtualSizedContainerAddingBeginCount === 0) {
      this._virtualSizedContainers.length = 0;
    }
  }
  addVirtualSizedContainer(container) {
    this._virtualSizedContainers.push(container);
  }
  endVirtualSizedContainerAdding() {
    if (--this._virtualSizedContainerAddingBeginCount === 0) {
      const count = this._virtualSizedContainers.length;
      if (count > 0) {
        this.fireBeforeVirtualRectingEvent(count);
        for (let i = 0; i < count; i++) {
          const container = this._virtualSizedContainers[i];
          container.notifyVirtualRectingRequired();
        }
        this.fireAfterVirtualRectingEvent();
        this._virtualSizedContainers.length = 0;
      }
    }
  }
  fireBeforeVirtualRectingEvent(count) {
    if (this.beforeVirtualRectingEvent !== void 0) {
      this.beforeVirtualRectingEvent(count);
    }
  }
  fireAfterVirtualRectingEvent() {
    if (this.afterVirtualRectingEvent !== void 0) {
      this.afterVirtualRectingEvent();
    }
  }
  createPopoutFromItemConfig(rootItemConfig, window2, parentId, indexInParent) {
    const layoutConfig = this.toConfig();
    const popoutLayoutConfig = {
      root: rootItemConfig,
      openPopouts: [],
      settings: layoutConfig.settings,
      dimensions: layoutConfig.dimensions,
      header: layoutConfig.header,
      window: window2,
      parentId,
      indexInParent,
      resolved: true
    };
    return this.createPopoutFromPopoutLayoutConfig(popoutLayoutConfig);
  }
  createPopoutFromPopoutLayoutConfig(config) {
    var _a, _b, _c, _d;
    const configWindow = config.window;
    const initialWindow = {
      left: (_a = configWindow.left) !== null && _a !== void 0 ? _a : globalThis.screenX || globalThis.screenLeft + 20,
      top: (_b = configWindow.top) !== null && _b !== void 0 ? _b : globalThis.screenY || globalThis.screenTop + 20,
      width: (_c = configWindow.width) !== null && _c !== void 0 ? _c : 500,
      height: (_d = configWindow.height) !== null && _d !== void 0 ? _d : 309
    };
    const browserPopout = new BrowserPopout(config, initialWindow, this);
    browserPopout.on("initialised", () => this.emit("windowOpened", browserPopout));
    browserPopout.on("closed", () => this.reconcilePopoutWindows());
    this._openPopouts.push(browserPopout);
    return browserPopout;
  }
  newDragSource(element, componentTypeOrItemConfigCallback, componentState, title) {
    const dragSource = new DragSource(this, element, [], componentTypeOrItemConfigCallback, componentState, title);
    this._dragSources.push(dragSource);
    return dragSource;
  }
  removeDragSource(dragSource) {
    removeFromArray(dragSource, this._dragSources);
    dragSource.destroy();
  }
  startComponentDrag(x, y, dragListener, componentItem, stack) {
    new DragProxy(x, y, dragListener, this, componentItem, stack);
  }
  focusComponent(item, suppressEvent = false) {
    item.focus(suppressEvent);
  }
  clearComponentFocus(suppressEvent = false) {
    this.setFocusedComponentItem(void 0, suppressEvent);
  }
  setFocusedComponentItem(item, suppressEvents = false) {
    if (item !== this._focusedComponentItem) {
      let newFocusedParentItem;
      if (item === void 0)
        ;
      else {
        newFocusedParentItem = item.parentItem;
      }
      if (this._focusedComponentItem !== void 0) {
        const oldFocusedItem = this._focusedComponentItem;
        this._focusedComponentItem = void 0;
        oldFocusedItem.setBlurred(suppressEvents);
        const oldFocusedParentItem = oldFocusedItem.parentItem;
        if (newFocusedParentItem === oldFocusedParentItem) {
          newFocusedParentItem = void 0;
        } else {
          oldFocusedParentItem.setFocusedValue(false);
        }
      }
      if (item !== void 0) {
        this._focusedComponentItem = item;
        item.setFocused(suppressEvents);
        if (newFocusedParentItem !== void 0) {
          newFocusedParentItem.setFocusedValue(true);
        }
      }
    }
  }
  createContentItemFromConfig(config, parent) {
    switch (config.type) {
      case ItemType.ground:
        throw new AssertError("LMCCIFC68871");
      case ItemType.row:
        return new RowOrColumn(false, this, config, parent);
      case ItemType.column:
        return new RowOrColumn(true, this, config, parent);
      case ItemType.stack:
        return new Stack(this, config, parent);
      case ItemType.component:
        return new ComponentItem(this, config, parent);
      default:
        throw new UnreachableCaseError("CCC913564", config.type, "Invalid Config Item type specified");
    }
  }
  setMaximisedStack(stack) {
    if (stack === void 0) {
      if (this._maximisedStack !== void 0) {
        this.processMinimiseMaximisedStack();
      }
    } else {
      if (stack !== this._maximisedStack) {
        if (this._maximisedStack !== void 0) {
          this.processMinimiseMaximisedStack();
        }
        this.processMaximiseStack(stack);
      }
    }
  }
  checkMinimiseMaximisedStack() {
    if (this._maximisedStack !== void 0) {
      this._maximisedStack.minimise();
    }
  }
  cleanupBeforeMaximisedStackDestroyed(event) {
    if (this._maximisedStack !== null && this._maximisedStack === event.target) {
      this._maximisedStack.off("beforeItemDestroyed", this._maximisedStackBeforeDestroyedListener);
      this._maximisedStack = void 0;
    }
  }
  closeWindow() {
    globalThis.setTimeout(() => globalThis.close(), 1);
  }
  getArea(x, y) {
    let matchingArea = null;
    let smallestSurface = Infinity;
    for (let i = 0; i < this._itemAreas.length; i++) {
      const area = this._itemAreas[i];
      if (x > area.x1 && x < area.x2 && y > area.y1 && y < area.y2 && smallestSurface > area.surface) {
        smallestSurface = area.surface;
        matchingArea = area;
      }
    }
    return matchingArea;
  }
  calculateItemAreas() {
    const allContentItems = this.getAllContentItems();
    const groundItem = this._groundItem;
    if (groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMCIAR44365");
    } else {
      if (allContentItems.length === 1) {
        const groundArea = groundItem.getElementArea();
        if (groundArea === null) {
          throw new UnexpectedNullError("LMCIARA44365");
        } else {
          this._itemAreas = [groundArea];
        }
        return;
      } else {
        if (groundItem.contentItems[0].isStack) {
          this._itemAreas = [];
        } else {
          this._itemAreas = groundItem.createSideAreas();
        }
        for (let i = 0; i < allContentItems.length; i++) {
          const stack = allContentItems[i];
          if (ContentItem.isStack(stack)) {
            const area = stack.getArea();
            if (area === null) {
              continue;
            } else {
              this._itemAreas.push(area);
              const stackContentAreaDimensions = stack.contentAreaDimensions;
              if (stackContentAreaDimensions === void 0) {
                throw new UnexpectedUndefinedError("LMCIASC45599");
              } else {
                const highlightArea = stackContentAreaDimensions.header.highlightArea;
                const surface = (highlightArea.x2 - highlightArea.x1) * (highlightArea.y2 - highlightArea.y1);
                const header = {
                  x1: highlightArea.x1,
                  x2: highlightArea.x2,
                  y1: highlightArea.y1,
                  y2: highlightArea.y2,
                  contentItem: stack,
                  surface
                };
                this._itemAreas.push(header);
              }
            }
          }
        }
      }
    }
  }
  checkLoadedLayoutMaximiseItem() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMCLLMI43432");
    } else {
      const configMaximisedItems = this._groundItem.getConfigMaximisedItems();
      if (configMaximisedItems.length > 0) {
        let item = configMaximisedItems[0];
        if (ContentItem.isComponentItem(item)) {
          const stack = item.parent;
          if (stack === null) {
            throw new UnexpectedNullError("LMXLLMI69999");
          } else {
            item = stack;
          }
        }
        if (!ContentItem.isStack(item)) {
          throw new AssertError("LMCLLMI19993");
        } else {
          item.maximise();
        }
      }
    }
  }
  processMaximiseStack(stack) {
    this._maximisedStack = stack;
    stack.on("beforeItemDestroyed", this._maximisedStackBeforeDestroyedListener);
    stack.element.classList.add("lm_maximised");
    stack.element.insertAdjacentElement("afterend", this._maximisePlaceholder);
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMMXI19993");
    } else {
      this._groundItem.element.prepend(stack.element);
      const {width, height} = getElementWidthAndHeight(this._containerElement);
      setElementWidth(stack.element, width);
      setElementHeight(stack.element, height);
      stack.updateSize();
      stack.focusActiveContentItem();
      this._maximisedStack.emit("maximised");
      this.emit("stateChanged");
    }
  }
  processMinimiseMaximisedStack() {
    if (this._maximisedStack === void 0) {
      throw new AssertError("LMMMS74422");
    } else {
      const stack = this._maximisedStack;
      if (stack.parent === null) {
        throw new UnexpectedNullError("LMMI13668");
      } else {
        stack.element.classList.remove("lm_maximised");
        this._maximisePlaceholder.insertAdjacentElement("afterend", stack.element);
        this._maximisePlaceholder.remove();
        stack.parent.updateSize();
        this._maximisedStack = void 0;
        stack.off("beforeItemDestroyed", this._maximisedStackBeforeDestroyedListener);
        stack.emit("minimised");
        this.emit("stateChanged");
      }
    }
  }
  reconcilePopoutWindows() {
    const openPopouts = [];
    for (let i = 0; i < this._openPopouts.length; i++) {
      if (this._openPopouts[i].getWindow().closed === false) {
        openPopouts.push(this._openPopouts[i]);
      } else {
        this.emit("windowClosed", this._openPopouts[i]);
      }
    }
    if (this._openPopouts.length !== openPopouts.length) {
      this._openPopouts = openPopouts;
      this.emit("stateChanged");
    }
  }
  getAllContentItems() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMGACI13130");
    } else {
      return this._groundItem.getAllContentItems();
    }
  }
  bindEvents() {
    if (this._isFullPage) {
      globalThis.addEventListener("resize", this._windowResizeListener, {passive: true});
    }
    globalThis.addEventListener("unload", this._windowUnloadListener, {passive: true});
    globalThis.addEventListener("beforeunload", this._windowUnloadListener, {passive: true});
  }
  createSubWindows() {
    for (let i = 0; i < this.layoutConfig.openPopouts.length; i++) {
      const popoutConfig = this.layoutConfig.openPopouts[i];
      this.createPopoutFromPopoutLayoutConfig(popoutConfig);
    }
  }
  processResizeWithDebounce() {
    if (this._resizeTimeoutId !== void 0) {
      clearTimeout(this._resizeTimeoutId);
    }
    this._resizeTimeoutId = setTimeout(() => this.updateSizeFromContainer(), 100);
  }
  setContainer() {
    var _a;
    const bodyElement = document.body;
    const containerElement = (_a = this._containerElement) !== null && _a !== void 0 ? _a : bodyElement;
    if (containerElement === bodyElement) {
      this._isFullPage = true;
      const documentElement = document.documentElement;
      documentElement.style.height = "100%";
      documentElement.style.margin = "0";
      documentElement.style.padding = "0";
      documentElement.style.overflow = "hidden";
      bodyElement.style.height = "100%";
      bodyElement.style.margin = "0";
      bodyElement.style.padding = "0";
      bodyElement.style.overflow = "hidden";
    }
    this._containerElement = containerElement;
  }
  onUnload() {
    this.destroy();
  }
  adjustColumnsResponsive() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMACR20883");
    } else {
      this._firstLoad = false;
      if (this.useResponsiveLayout() && !this._updatingColumnsResponsive && this._groundItem.contentItems.length > 0 && this._groundItem.contentItems[0].isRow) {
        if (this._groundItem === void 0 || this._width === null) {
          throw new UnexpectedUndefinedError("LMACR77412");
        } else {
          const columnCount = this._groundItem.contentItems[0].contentItems.length;
          if (columnCount <= 1) {
            return;
          } else {
            const minItemWidth = this.layoutConfig.dimensions.minItemWidth;
            const totalMinWidth = columnCount * minItemWidth;
            if (totalMinWidth <= this._width) {
              return;
            } else {
              this._updatingColumnsResponsive = true;
              const finalColumnCount = Math.max(Math.floor(this._width / minItemWidth), 1);
              const stackColumnCount = columnCount - finalColumnCount;
              const rootContentItem = this._groundItem.contentItems[0];
              const allStacks = this.getAllStacks();
              if (allStacks.length === 0) {
                throw new AssertError("LMACRS77413");
              } else {
                const firstStackContainer = allStacks[0];
                for (let i = 0; i < stackColumnCount; i++) {
                  const column = rootContentItem.contentItems[rootContentItem.contentItems.length - 1];
                  this.addChildContentItemsToContainer(firstStackContainer, column);
                }
                this._updatingColumnsResponsive = false;
              }
            }
          }
        }
      }
    }
  }
  useResponsiveLayout() {
    const settings = this.layoutConfig.settings;
    const alwaysResponsiveMode = settings.responsiveMode === ResponsiveMode.always;
    const onLoadResponsiveModeAndFirst = settings.responsiveMode === ResponsiveMode.onload && this._firstLoad;
    return alwaysResponsiveMode || onLoadResponsiveModeAndFirst;
  }
  addChildContentItemsToContainer(container, node) {
    const contentItems = node.contentItems;
    if (node instanceof Stack) {
      for (let i = 0; i < contentItems.length; i++) {
        const item = contentItems[i];
        node.removeChild(item, true);
        container.addChild(item);
      }
    } else {
      for (let i = 0; i < contentItems.length; i++) {
        const item = contentItems[i];
        this.addChildContentItemsToContainer(container, item);
      }
    }
  }
  getAllStacks() {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMFASC52778");
    } else {
      const stacks = [];
      this.findAllStacksRecursive(stacks, this._groundItem);
      return stacks;
    }
  }
  findFirstContentItemType(type) {
    if (this._groundItem === void 0) {
      throw new UnexpectedUndefinedError("LMFFCIT82446");
    } else {
      return this.findFirstContentItemTypeRecursive(type, this._groundItem);
    }
  }
  findFirstContentItemTypeRecursive(type, node) {
    const contentItems = node.contentItems;
    const contentItemCount = contentItems.length;
    if (contentItemCount === 0) {
      return void 0;
    } else {
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        if (contentItem.type === type) {
          return contentItem;
        }
      }
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        const foundContentItem = this.findFirstContentItemTypeRecursive(type, contentItem);
        if (foundContentItem !== void 0) {
          return foundContentItem;
        }
      }
      return void 0;
    }
  }
  findFirstContentItemTypeByIdRecursive(type, id, node) {
    const contentItems = node.contentItems;
    const contentItemCount = contentItems.length;
    if (contentItemCount === 0) {
      return void 0;
    } else {
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        if (contentItem.type === type && contentItem.id === id) {
          return contentItem;
        }
      }
      for (let i = 0; i < contentItemCount; i++) {
        const contentItem = contentItems[i];
        const foundContentItem = this.findFirstContentItemTypeByIdRecursive(type, id, contentItem);
        if (foundContentItem !== void 0) {
          return foundContentItem;
        }
      }
      return void 0;
    }
  }
  findAllStacksRecursive(stacks, node) {
    const contentItems = node.contentItems;
    for (let i = 0; i < contentItems.length; i++) {
      const item = contentItems[i];
      if (item instanceof Stack) {
        stacks.push(item);
      } else {
        if (!item.isComponent) {
          this.findAllStacksRecursive(stacks, item);
        }
      }
    }
  }
  findFirstLocation(selectors) {
    const count = selectors.length;
    for (let i = 0; i < count; i++) {
      const selector = selectors[i];
      const location2 = this.findLocation(selector);
      if (location2 !== void 0) {
        return location2;
      }
    }
    return void 0;
  }
  findLocation(selector) {
    const selectorIndex = selector.index;
    switch (selector.typeId) {
      case 0: {
        if (this._focusedComponentItem === void 0) {
          return void 0;
        } else {
          const parentItem = this._focusedComponentItem.parentItem;
          const parentContentItems = parentItem.contentItems;
          const parentContentItemCount = parentContentItems.length;
          if (selectorIndex === void 0) {
            return {parentItem, index: parentContentItemCount};
          } else {
            const focusedIndex = parentContentItems.indexOf(this._focusedComponentItem);
            const index = focusedIndex + selectorIndex;
            if (index < 0 || index > parentContentItemCount) {
              return void 0;
            } else {
              return {parentItem, index};
            }
          }
        }
      }
      case 1: {
        if (this._focusedComponentItem === void 0) {
          return void 0;
        } else {
          const parentItem = this._focusedComponentItem.parentItem;
          return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
        }
      }
      case 2: {
        const parentItem = this.findFirstContentItemType(ItemType.stack);
        if (parentItem === void 0) {
          return void 0;
        } else {
          return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
        }
      }
      case 3: {
        let parentItem = this.findFirstContentItemType(ItemType.row);
        if (parentItem !== void 0) {
          return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
        } else {
          parentItem = this.findFirstContentItemType(ItemType.column);
          if (parentItem !== void 0) {
            return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
          } else {
            return void 0;
          }
        }
      }
      case 4: {
        const parentItem = this.findFirstContentItemType(ItemType.row);
        if (parentItem === void 0) {
          return void 0;
        } else {
          return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
        }
      }
      case 5: {
        const parentItem = this.findFirstContentItemType(ItemType.column);
        if (parentItem === void 0) {
          return void 0;
        } else {
          return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
        }
      }
      case 6: {
        if (this._groundItem === void 0) {
          throw new UnexpectedUndefinedError("LMFLRIF18244");
        } else {
          if (this.rootItem !== void 0) {
            return void 0;
          } else {
            if (selectorIndex === void 0 || selectorIndex === 0)
              return {parentItem: this._groundItem, index: 0};
            else {
              return void 0;
            }
          }
        }
      }
      case 7: {
        if (this._groundItem === void 0) {
          throw new UnexpectedUndefinedError("LMFLF18244");
        } else {
          const groundContentItems = this._groundItem.contentItems;
          if (groundContentItems.length === 0) {
            if (selectorIndex === void 0 || selectorIndex === 0)
              return {parentItem: this._groundItem, index: 0};
            else {
              return void 0;
            }
          } else {
            const parentItem = groundContentItems[0];
            return this.tryCreateLocationFromParentItem(parentItem, selectorIndex);
          }
        }
      }
    }
  }
  tryCreateLocationFromParentItem(parentItem, selectorIndex) {
    const parentContentItems = parentItem.contentItems;
    const parentContentItemCount = parentContentItems.length;
    if (selectorIndex === void 0) {
      return {parentItem, index: parentContentItemCount};
    } else {
      if (selectorIndex < 0 || selectorIndex > parentContentItemCount) {
        return void 0;
      } else {
        return {parentItem, index: selectorIndex};
      }
    }
  }
}
(function(LayoutManager2) {
  function createMaximisePlaceElement(document2) {
    const element = document2.createElement("div");
    element.classList.add("lm_maximise_place");
    return element;
  }
  LayoutManager2.createMaximisePlaceElement = createMaximisePlaceElement;
  function createTabDropPlaceholderElement(document2) {
    const element = document2.createElement("div");
    element.classList.add("lm_drop_tab_placeholder");
    return element;
  }
  LayoutManager2.createTabDropPlaceholderElement = createTabDropPlaceholderElement;
  LayoutManager2.defaultLocationSelectors = [
    {typeId: 1, index: void 0},
    {typeId: 2, index: void 0},
    {typeId: 3, index: void 0},
    {typeId: 7, index: void 0}
  ];
  LayoutManager2.afterFocusedItemIfPossibleLocationSelectors = [
    {typeId: 0, index: 1},
    {typeId: 2, index: void 0},
    {typeId: 3, index: void 0},
    {typeId: 7, index: void 0}
  ];
})(LayoutManager || (LayoutManager = {}));
class VirtualLayout extends LayoutManager {
  constructor(configOrOptionalContainer, containerOrBindComponentEventHandler, unbindComponentEventHandler, skipInit) {
    super(VirtualLayout.createLayoutManagerConstructorParameters(configOrOptionalContainer, containerOrBindComponentEventHandler));
    this._bindComponentEventHanlderPassedInConstructor = false;
    this._creationTimeoutPassed = false;
    if (containerOrBindComponentEventHandler !== void 0) {
      if (typeof containerOrBindComponentEventHandler === "function") {
        this.bindComponentEvent = containerOrBindComponentEventHandler;
        this._bindComponentEventHanlderPassedInConstructor = true;
        if (unbindComponentEventHandler !== void 0) {
          this.unbindComponentEvent = unbindComponentEventHandler;
        }
      }
    }
    if (!this._bindComponentEventHanlderPassedInConstructor) {
      if (this.isSubWindow) {
        if (this._constructorOrSubWindowLayoutConfig === void 0) {
          throw new UnexpectedUndefinedError("VLC98823");
        } else {
          const resolvedLayoutConfig = LayoutConfig.resolve(this._constructorOrSubWindowLayoutConfig);
          this.layoutConfig = Object.assign(Object.assign({}, resolvedLayoutConfig), {root: void 0});
        }
      }
    }
    if (skipInit !== true) {
      if (!this.deprecatedConstructor) {
        this.init();
      }
    }
  }
  destroy() {
    this.bindComponentEvent = void 0;
    this.unbindComponentEvent = void 0;
    super.destroy();
  }
  init() {
    if (!this._bindComponentEventHanlderPassedInConstructor && (document.readyState === "loading" || document.body === null)) {
      document.addEventListener("DOMContentLoaded", () => this.init(), {passive: true});
      return;
    }
    if (!this._bindComponentEventHanlderPassedInConstructor && this.isSubWindow === true && !this._creationTimeoutPassed) {
      setTimeout(() => this.init(), 7);
      this._creationTimeoutPassed = true;
      return;
    }
    if (this.isSubWindow === true) {
      if (!this._bindComponentEventHanlderPassedInConstructor) {
        this.clearHtmlAndAdjustStylesForSubWindow();
      }
      window.__glInstance = this;
    }
    super.init();
  }
  clearHtmlAndAdjustStylesForSubWindow() {
    const headElement = document.head;
    const appendNodeLists = new Array(4);
    appendNodeLists[0] = document.querySelectorAll("body link");
    appendNodeLists[1] = document.querySelectorAll("body style");
    appendNodeLists[2] = document.querySelectorAll("template");
    appendNodeLists[3] = document.querySelectorAll(".gl_keep");
    for (let listIdx = 0; listIdx < appendNodeLists.length; listIdx++) {
      const appendNodeList = appendNodeLists[listIdx];
      for (let nodeIdx = 0; nodeIdx < appendNodeList.length; nodeIdx++) {
        const node = appendNodeList[nodeIdx];
        headElement.appendChild(node);
      }
    }
    const bodyElement = document.body;
    bodyElement.innerHTML = "";
    bodyElement.style.visibility = "visible";
    this.checkAddDefaultPopinButton();
    const x = document.body.offsetHeight;
  }
  checkAddDefaultPopinButton() {
    if (this.layoutConfig.settings.popInOnClose) {
      return false;
    } else {
      const popInButtonElement = document.createElement("div");
      popInButtonElement.classList.add("lm_popin");
      popInButtonElement.setAttribute("title", this.layoutConfig.header.dock);
      const iconElement = document.createElement("div");
      iconElement.classList.add("lm_icon");
      const bgElement = document.createElement("div");
      bgElement.classList.add("lm_bg");
      popInButtonElement.appendChild(iconElement);
      popInButtonElement.appendChild(bgElement);
      popInButtonElement.addEventListener("click", () => this.emit("popIn"));
      document.body.appendChild(popInButtonElement);
      return true;
    }
  }
  bindComponent(container, itemConfig) {
    if (this.bindComponentEvent !== void 0) {
      const bindableComponent = this.bindComponentEvent(container, itemConfig);
      return bindableComponent;
    } else {
      if (this.getComponentEvent !== void 0) {
        return {
          virtual: false,
          component: this.getComponentEvent(container, itemConfig)
        };
      } else {
        const text = i18nStrings[2];
        const message = `${text}: ${JSON.stringify(itemConfig)}`;
        throw new BindError(message);
      }
    }
  }
  unbindComponent(container, virtual, component) {
    if (virtual) {
      if (this.unbindComponentEvent !== void 0) {
        this.unbindComponentEvent(container);
      }
    } else {
      if (this.releaseComponentEvent !== void 0) {
        if (component === void 0) {
          throw new UnexpectedUndefinedError("VCUCRCU333998");
        } else {
          this.releaseComponentEvent(container, component);
        }
      }
    }
  }
}
(function(VirtualLayout2) {
  let subWindowChecked = false;
  function createLayoutManagerConstructorParameters(configOrOptionalContainer, containerOrBindComponentEventHandler) {
    const windowConfigKey = subWindowChecked ? null : new URL(document.location.href).searchParams.get("gl-window");
    subWindowChecked = true;
    const isSubWindow = windowConfigKey !== null;
    let containerElement;
    let config;
    if (windowConfigKey !== null) {
      const windowConfigStr = localStorage.getItem(windowConfigKey);
      if (windowConfigStr === null) {
        throw new Error("Null gl-window Config");
      }
      localStorage.removeItem(windowConfigKey);
      const minifiedWindowConfig = JSON.parse(windowConfigStr);
      const resolvedConfig = ResolvedLayoutConfig.unminifyConfig(minifiedWindowConfig);
      config = LayoutConfig.fromResolved(resolvedConfig);
      if (configOrOptionalContainer instanceof HTMLElement) {
        containerElement = configOrOptionalContainer;
      }
    } else {
      if (configOrOptionalContainer === void 0) {
        config = void 0;
      } else {
        if (configOrOptionalContainer instanceof HTMLElement) {
          config = void 0;
          containerElement = configOrOptionalContainer;
        } else {
          config = configOrOptionalContainer;
        }
      }
      if (containerElement === void 0) {
        if (containerOrBindComponentEventHandler instanceof HTMLElement) {
          containerElement = containerOrBindComponentEventHandler;
        }
      }
    }
    return {
      constructorOrSubWindowLayoutConfig: config,
      isSubWindow,
      containerElement
    };
  }
  VirtualLayout2.createLayoutManagerConstructorParameters = createLayoutManagerConstructorParameters;
})(VirtualLayout || (VirtualLayout = {}));
class GoldenLayout extends VirtualLayout {
  constructor(configOrOptionalContainer, containerOrBindComponentEventHandler, unbindComponentEventHandler) {
    super(configOrOptionalContainer, containerOrBindComponentEventHandler, unbindComponentEventHandler, true);
    this._componentTypesMap = new Map();
    this._virtuableComponentMap = new Map();
    this._containerVirtualRectingRequiredEventListener = (container, width, height) => this.handleContainerVirtualRectingRequiredEvent(container, width, height);
    this._containerVirtualVisibilityChangeRequiredEventListener = (container, visible) => this.handleContainerVirtualVisibilityChangeRequiredEvent(container, visible);
    this._containerVirtualZIndexChangeRequiredEventListener = (container, logicalZIndex, defaultZIndex) => this.handleContainerVirtualZIndexChangeRequiredEvent(container, logicalZIndex, defaultZIndex);
    if (!this.deprecatedConstructor) {
      this.init();
    }
  }
  registerComponent(name, componentConstructorOrFactoryFtn, virtual = false) {
    if (typeof componentConstructorOrFactoryFtn !== "function") {
      throw new ApiError("registerComponent() componentConstructorOrFactoryFtn parameter is not a function");
    } else {
      if (componentConstructorOrFactoryFtn.hasOwnProperty("prototype")) {
        const componentConstructor = componentConstructorOrFactoryFtn;
        this.registerComponentConstructor(name, componentConstructor, virtual);
      } else {
        const componentFactoryFtn = componentConstructorOrFactoryFtn;
        this.registerComponentFactoryFunction(name, componentFactoryFtn, virtual);
      }
    }
  }
  registerComponentConstructor(typeName, componentConstructor, virtual = false) {
    if (typeof componentConstructor !== "function") {
      throw new Error(i18nStrings[1]);
    }
    const existingComponentType = this._componentTypesMap.get(typeName);
    if (existingComponentType !== void 0) {
      throw new BindError(`${i18nStrings[3]}: ${typeName}`);
    }
    this._componentTypesMap.set(typeName, {
      constructor: componentConstructor,
      factoryFunction: void 0,
      virtual
    });
  }
  registerComponentFactoryFunction(typeName, componentFactoryFunction, virtual = false) {
    if (typeof componentFactoryFunction !== "function") {
      throw new BindError("Please register a constructor function");
    }
    const existingComponentType = this._componentTypesMap.get(typeName);
    if (existingComponentType !== void 0) {
      throw new BindError(`${i18nStrings[3]}: ${typeName}`);
    }
    this._componentTypesMap.set(typeName, {
      constructor: void 0,
      factoryFunction: componentFactoryFunction,
      virtual
    });
  }
  registerComponentFunction(callback) {
    this.registerGetComponentConstructorCallback(callback);
  }
  registerGetComponentConstructorCallback(callback) {
    if (typeof callback !== "function") {
      throw new Error("Please register a callback function");
    }
    if (this._getComponentConstructorFtn !== void 0) {
      console.warn("Multiple component functions are being registered.  Only the final registered function will be used.");
    }
    this._getComponentConstructorFtn = callback;
  }
  getRegisteredComponentTypeNames() {
    const typeNamesIterableIterator = this._componentTypesMap.keys();
    return Array.from(typeNamesIterableIterator);
  }
  getComponentInstantiator(config) {
    let instantiator;
    const typeName = ResolvedComponentItemConfig.resolveComponentTypeName(config);
    if (typeName !== void 0) {
      instantiator = this._componentTypesMap.get(typeName);
    }
    if (instantiator === void 0) {
      if (this._getComponentConstructorFtn !== void 0) {
        instantiator = {
          constructor: this._getComponentConstructorFtn(config),
          factoryFunction: void 0,
          virtual: false
        };
      }
    }
    return instantiator;
  }
  bindComponent(container, itemConfig) {
    let instantiator;
    const typeName = ResolvedComponentItemConfig.resolveComponentTypeName(itemConfig);
    if (typeName !== void 0) {
      instantiator = this._componentTypesMap.get(typeName);
    }
    if (instantiator === void 0) {
      if (this._getComponentConstructorFtn !== void 0) {
        instantiator = {
          constructor: this._getComponentConstructorFtn(itemConfig),
          factoryFunction: void 0,
          virtual: false
        };
      }
    }
    let result;
    if (instantiator !== void 0) {
      const virtual = instantiator.virtual;
      let componentState;
      if (itemConfig.componentState === void 0) {
        componentState = void 0;
      } else {
        componentState = deepExtendValue({}, itemConfig.componentState);
      }
      let component;
      const componentConstructor = instantiator.constructor;
      if (componentConstructor !== void 0) {
        component = new componentConstructor(container, componentState, virtual);
      } else {
        const factoryFunction = instantiator.factoryFunction;
        if (factoryFunction !== void 0) {
          component = factoryFunction(container, componentState, virtual);
        } else {
          throw new AssertError("LMBCFFU10008");
        }
      }
      if (virtual) {
        if (component === void 0) {
          throw new UnexpectedUndefinedError("GLBCVCU988774");
        } else {
          const virtuableComponent = component;
          const componentRootElement = virtuableComponent.rootHtmlElement;
          if (componentRootElement === void 0) {
            throw new BindError(`${i18nStrings[5]}: ${typeName}`);
          } else {
            ensureElementPositionAbsolute(componentRootElement);
            this.container.appendChild(componentRootElement);
            this._virtuableComponentMap.set(container, virtuableComponent);
            container.virtualRectingRequiredEvent = this._containerVirtualRectingRequiredEventListener;
            container.virtualVisibilityChangeRequiredEvent = this._containerVirtualVisibilityChangeRequiredEventListener;
            container.virtualZIndexChangeRequiredEvent = this._containerVirtualZIndexChangeRequiredEventListener;
          }
        }
      }
      result = {
        virtual: instantiator.virtual,
        component
      };
    } else {
      result = super.bindComponent(container, itemConfig);
    }
    return result;
  }
  unbindComponent(container, virtual, component) {
    const virtuableComponent = this._virtuableComponentMap.get(container);
    if (virtuableComponent === void 0) {
      super.unbindComponent(container, virtual, component);
    } else {
      const componentRootElement = virtuableComponent.rootHtmlElement;
      if (componentRootElement === void 0) {
        throw new AssertError("GLUC77743", container.title);
      } else {
        this.container.removeChild(componentRootElement);
        this._virtuableComponentMap.delete(container);
      }
    }
  }
  fireBeforeVirtualRectingEvent(count) {
    this._goldenLayoutBoundingClientRect = this.container.getBoundingClientRect();
    super.fireBeforeVirtualRectingEvent(count);
  }
  handleContainerVirtualRectingRequiredEvent(container, width, height) {
    const virtuableComponent = this._virtuableComponentMap.get(container);
    if (virtuableComponent === void 0) {
      throw new UnexpectedUndefinedError("GLHCSCE55933");
    } else {
      const rootElement = virtuableComponent.rootHtmlElement;
      if (rootElement === void 0) {
        throw new BindError(i18nStrings[4] + " " + container.title);
      } else {
        const containerBoundingClientRect = container.element.getBoundingClientRect();
        const left = containerBoundingClientRect.left - this._goldenLayoutBoundingClientRect.left;
        rootElement.style.left = numberToPixels(left);
        const top = containerBoundingClientRect.top - this._goldenLayoutBoundingClientRect.top;
        rootElement.style.top = numberToPixels(top);
        setElementWidth(rootElement, width);
        setElementHeight(rootElement, height);
      }
    }
  }
  handleContainerVirtualVisibilityChangeRequiredEvent(container, visible) {
    const virtuableComponent = this._virtuableComponentMap.get(container);
    if (virtuableComponent === void 0) {
      throw new UnexpectedUndefinedError("GLHCVVCRE55934");
    } else {
      const rootElement = virtuableComponent.rootHtmlElement;
      if (rootElement === void 0) {
        throw new BindError(i18nStrings[4] + " " + container.title);
      } else {
        setElementDisplayVisibility(rootElement, visible);
      }
    }
  }
  handleContainerVirtualZIndexChangeRequiredEvent(container, logicalZIndex, defaultZIndex) {
    const virtuableComponent = this._virtuableComponentMap.get(container);
    if (virtuableComponent === void 0) {
      throw new UnexpectedUndefinedError("GLHCVZICRE55935");
    } else {
      const rootElement = virtuableComponent.rootHtmlElement;
      if (rootElement === void 0) {
        throw new BindError(i18nStrings[4] + " " + container.title);
      } else {
        rootElement.style.zIndex = defaultZIndex;
      }
    }
  }
}
export {ApiError, BindError, BrowserPopout, ComponentContainer, ComponentItem, ComponentItemConfig, ConfigurationError, ContentItem, DragSource, EventEmitter, EventHub, ExternalError, GoldenLayout, Header, HeaderedItemConfig, I18nStrings, ItemConfig, ItemType, JsonValue, LayoutConfig, LayoutManager, LogicalZIndex, PopoutBlockedError, PopoutLayoutConfig, ResolvedComponentItemConfig, ResolvedGroundItemConfig, ResolvedHeaderedItemConfig, ResolvedItemConfig, ResolvedLayoutConfig, ResolvedPopoutLayoutConfig, ResolvedRootItemConfig, ResolvedRowOrColumnItemConfig, ResolvedStackItemConfig, ResponsiveMode, RootItemConfig, RowOrColumn, RowOrColumnItemConfig, Side, Stack, StackItemConfig, StyleConstants, Tab, VirtualLayout, WidthOrHeightPropertyName, i18nStrings};
export default null;
