#!/bin/bash

# dycolor-theme-manager
# 动态壁纸配色管理器 for KDE6
# 监听壁纸切换，生成pywal配色，注入KDE颜色主题

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/dycolor-config.ini"
LOG_FILE="${SCRIPT_DIR}/dycolor.log"
PID_FILE="${SCRIPT_DIR}/dycolor.pid"
POLL_INTERVAL_SECONDS=1800  # 默认30分钟
# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
KDE_COLOR_TAG="1"
KDE_COLOR_FILE1=""
KDE_COLOR_FILE2=""
# 日志函数
log() {
    local level="$1"
    local message="$2"
    local timestamp="$(date '+%Y-%m-%d %H:%M:%S')"
    echo -e "[${timestamp}] [${level}] ${message}" >> "$LOG_FILE"
}

log_info() {
    log "INFO" "$1"
}

log_warn() {
    log "WARN" "${YELLOW}$1${NC}"
}

log_error() {
    log "ERROR" "${RED}$1${NC}"
}

log_success() {
    log "SUCCESS" "${GREEN}$1${NC}"
}

# 检查依赖
check_dependencies() {
    local missing=()

    # 检查必需工具
    for cmd in wal plasma-apply-colorscheme; do
        if ! command -v "$cmd" &> /dev/null; then
            missing+=("$cmd")
        fi
    done

    # 检查可选工具（用于D-Bus监控）
    if ! command -v qdbus6 &> /dev/null && ! command -v dbus-monitor &> /dev/null; then
        log_warn "qdbus6或dbus-monitor未找到，将使用纯轮询模式监控壁纸变化"
    else
        log_info "D-Bus监控工具可用"
    fi

    if [ ${#missing[@]} -gt 0 ]; then
        log_error "缺少必需依赖: ${missing[*]}"
        log_error "请安装: sudo pacman -S pywal plasma-workspace"
        return 1
    fi

    log_info "所有依赖已安装"
    return 0
}

# 读取程序配置文件
read_config() {
    if [ ! -f "$CONFIG_FILE" ]; then
        log_error "配置文件不存在: $CONFIG_FILE"
        log_error "请创建配置文件，第一行指定.colors文件路径"
        return 1
    fi

    # 读取颜色映射文件（.colors文件路径）
    KDE_COLOR_FILE1="$(head -1 "$CONFIG_FILE" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"
    KDE_COLOR_FILE2="$(sed -n '2p' "$CONFIG_FILE" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')"

    # 读取程序配置
    declare -gA COLOR_MAP
    local line_num=0
    while IFS= read -r line; do
        ((line_num++))
        # 跳过第一行
        [ $line_num -eq 1 ] && continue
        [[ "$line" =~ ^[[:space:]]*$ ]] && continue
        [[ "$line" =~ ^[[:space:]]*# ]] && continue

        # 检查轮询间隔配置（秒）
        if [[ "$line" =~ ^\[dycolor\]poll_interval_seconds=([0-9]+)$ ]]; then
            local interval="${BASH_REMATCH[1]}"
            if (( interval > 0 )); then
                POLL_INTERVAL_SECONDS="$interval"
                log_info "配置轮询间隔: $POLL_INTERVAL_SECONDS 秒"
            else
                log_warn "轮询间隔必须大于0: $interval，使用默认值: $POLL_INTERVAL_SECONDS 秒"
            fi
            continue
        fi


    done < "$CONFIG_FILE"

    log_info "轮询间隔: $POLL_INTERVAL_SECONDS 秒"
    return 0
}

# 获取当前壁纸路径
get_current_wallpaper() {
    # 优先使用D-Bus方法获取壁纸路径
    local info=$(qdbus6 --session org.kde.plasmashell /PlasmaShell \
        org.kde.PlasmaShell.wallpaper 0 2>&1)
    local image_line=$(echo "$info" | grep "^Image:")

    if [[ -n "$image_line" ]]; then
        local wallpaper_path="${image_line#Image: file://}"
        echo "$wallpaper_path"
        return 0
    fi

    # D-Bus方法失败时，回退到配置文件方法
    local config_file="$HOME/.config/plasma-org.kde.plasma.desktop-appletsrc"
    if [ ! -f "$config_file" ]; then
        return 1
    fi

    # 查找幻灯片放映的当前图片
    local wallpaper_line=$(grep -A 5 '\[Containments\]\[61\]\[Wallpaper\]\[org\.kde\.slideshow\]\[General\]' "$config_file" | grep '^Image=')

    if [ -z "$wallpaper_line" ]; then
        # 尝试其他可能的容器ID
        for container_id in {1..100}; do
            wallpaper_line=$(grep -A 5 "\[Containments\]\[${container_id}\]\[Wallpaper\]\[org\.kde\.slideshow\]\[General\]" "$config_file" | grep '^Image=')
            [ -n "$wallpaper_line" ] && break
        done
    fi

    if [ -z "$wallpaper_line" ]; then
        return 1
    fi

    # 提取路径: 移除 file:// 前缀
    local wallpaper_path="${wallpaper_line#Image=file://}"
    echo "$wallpaper_path"
    return 0
}

# 运行pywal生成配色
run_pywal() {
    local wallpaper_path="$1"

    if [ ! -f "$wallpaper_path" ]; then
        log_error "壁纸文件不存在: $wallpaper_path"
        return 1
    fi

    log_info "运行pywal处理壁纸: $wallpaper_path"
    wal -i "$wallpaper_path" -n
    log_success "pywal配色生成完成"
    return 0
}



# 更新.colors文件
update_colors_file() {
   PROGRAM_NAME="dy_config_loader"
   SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
   UPDATE_PROGRAM="$SCRIPT_DIR/$PROGRAM_NAME"
   COLOR_CONFIG_FILE="$SCRIPT_DIR/color-config.txt"
   if [ $KDE_COLOR_TAG -eq "1" ]; then
       "$UPDATE_PROGRAM" "$COLOR_CONFIG_FILE" "$KDE_COLOR_FILE1"
       else
         "$UPDATE_PROGRAM" "$COLOR_CONFIG_FILE" "$KDE_COLOR_FILE2"
   fi
   return 0
}

# 应用颜色主题
apply_color_scheme() {
    if [ $KDE_COLOR_TAG -eq "1" ]; then
           COLORS_FILE="$KDE_COLOR_FILE1"
           else
             COLORS_FILE="$KDE_COLOR_FILE2"
       fi
    local colors_file="$COLORS_FILE"
    local theme_name="$(basename "$colors_file" .colors)"

    log_info "应用颜色主题: $theme_name"
    log_info "应用颜色文件 $colors_file"

    if ! plasma-apply-colorscheme "$colors_file" 2>&1 | tee -a "$LOG_FILE"; then
#        log_error "应用颜色主题失败"
        return 1
    fi

#    log_success "颜色主题应用成功"

    # 发送KConfig更改通知，强制应用重新读取配置
    # if command -v qdbus6 &> /dev/null; then
    #     log_info "发送KConfig更改通知，强制应用更新颜色配置 (Plasma 6.6)..."

    #     # 尝试多种KDE配置通知方法
    #     local notified=0

    #     # 辅助函数：检测DBus调用是否成功
    #     dbus_call_success() {
    #         local output
    #         output="$($@ 2>&1)"
    #         local exit_code=$?
    #         echo "$output" | tee -a "$LOG_FILE" > /dev/null
    #         # 检查是否有错误信息
    #         if echo "$output" | grep -q "Error\|does not exist\|No such\|invalid\|Failed"; then
    #             return 1
    #         fi
    #         return $exit_code
    #     }

        # # 方法1: KWin重新配置 - 已知在Plasma 6上工作
        # log_info "尝试方法1: KWin重新配置"
        # if dbus_call_success qdbus6 org.kde.KWin /KWin org.kde.KWin.reconfigure; then
        #     ((notified++))
        #     log_info "✓ KWin重新配置成功"
        # else
        #     log_warn "KWin reconfigure调用失败"
        # fi

        # # 方法2: 通知Plasma Shell评估脚本（Plasma 6方法）
        # log_info "尝试方法2: Plasma Shell评估脚本"
        # if dbus_call_success qdbus6 org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScripts; then
        #     ((notified++))
        #     log_info "✓ Plasma Shell脚本重新评估成功"
        # else
        #     # 尝试refreshTheme方法
        #     log_info "  尝试refreshTheme方法"
        #     if dbus_call_success qdbus6 org.kde.plasmashell /PlasmaShell org.kde.plasmashell.refreshTheme; then
        #         ((notified++))
        #         log_info "✓ Plasma Shell主题刷新成功"
        #     else
        #         log_warn "Plasma Shell主题更新调用失败"
        #     fi
        # fi

        # # 方法3: 通过kded6模块重新加载颜色配置
        # log_info "尝试方法3: 通过kded6重新加载颜色配置"
        # # 尝试重新加载kded6的颜色相关模块
        # if dbus_call_success qdbus6 org.kde.kded6 /kded org.kde.kded6.reloadModule -- "color" 2>/dev/null; then
        #     ((notified++))
        #     log_info "✓ kded6颜色模块重新加载成功"
        # elif dbus_call_success qdbus6 org.kde.kded6 /kded org.kde.kded6.loadModule -- "colors" 2>/dev/null; then
        #     ((notified++))
        #     log_info "✓ kded6颜色模块加载成功"
        # else
        #     log_info "kded6直接调用失败，尝试其他方法"
        # fi

        # 方法4: 发送DBus信号通知配置更改（Plasma 6风格）
        # log_info "尝试方法4: 发送DBus配置更改信号"
        # if command -v dbus-send &> /dev/null; then
        #     # 尝试发送通用配置更改信号
        #     if dbus-send --session --type=signal \
        #         --dest=org.kde.KWin \
        #         /KWin \
        #         org.kde.KWin.configChanged 2>&1 | tee -a "$LOG_FILE" | grep -q "Error\|Failed\|invalid"; then
        #         log_warn "DBus配置更改信号发送失败"
        #     else
        #         ((notified++))
        #         log_info "✓ DBus配置更改信号发送成功"
        #     fi
        # else
        #     log_info "跳过方法4: dbus-send命令未找到"
        # fi

        # # 方法5: 通过plasmashell.accentColor服务更新颜色（Plasma 6新特性）
        # log_info "尝试方法5: 更新accentColor服务"
        # # 尝试直接调用setAccentColor方法
        # if dbus_call_success qdbus6 org.kde.plasmashell.accentColor /AccentColor org.kde.plasmashell.accentColor.setAccentColor 0; then
        #     ((notified++))
        #     log_info "✓ accentColor服务更新成功（触发重新读取配置）"
        # else
        #     log_info "accentColor服务调用失败（可能是预期行为）"
        # fi

        # 方法6: 通知KSMServer重新加载配置（会话管理）
        # log_info "尝试方法6: 通知KSMServer"
        # if dbus_call_success qdbus6 org.kde.ksmserver /KSMServer org.kde.KSMServerInterface.reloadConfiguration; then
        #     ((notified++))
        #     log_info "✓ KSMServer重新加载配置成功"
        # else
        #     log_info "KSMServer调用失败（可能是预期行为）"
        # fi

        # 方法7: 使用kreadconfig6/kwriteconfig6触发配置重读
    #     log_info "尝试方法7: 触发配置重读"
    #     if command -v kwriteconfig6 &> /dev/null && command -v kreadconfig6 &> /dev/null; then
    #         # 写入临时配置值触发重读
    #         local temp_key="dycolor_trigger_$(date +%s)"
    #         local temp_value="temp_$(date +%s)"

    #         # 执行配置操作，输出重定向到日志
    #         if kwriteconfig6 --file kdeglobals --group "General" --key "$temp_key" "$temp_value" >> "$LOG_FILE" 2>&1 && \
    #            kreadconfig6 --file kdeglobals --group "General" --key "$temp_key" >> "$LOG_FILE" 2>&1; then
    #             # 删除临时键
    #             kwriteconfig6 --file kdeglobals --group "General" --key "$temp_key" --delete >> "$LOG_FILE" 2>&1
    #             ((notified++))
    #             log_info "✓ 配置读写触发成功"
    #         else
    #             log_warn "配置读写触发失败"
    #         fi
    #     else
    #         log_info "跳过方法7: kwriteconfig6/kreadconfig6未找到"
    #     fi

    #     if [ $notified -eq 0 ]; then
    #         log_warn "所有KConfig通知方法都失败，应用可能需要重启才能显示新颜色"
    #     elif [ $notified -lt 3 ]; then
    #         log_warn "KConfig通知部分成功 ($notified 个方法)，某些应用可能需要重启才能显示新颜色"
    #     else
    #         log_success "KConfig通知发送完成 ($notified 个方法成功)"
    #     fi
    # else
    #     log_warn "qdbus6未找到，无法发送KConfig更改通知"
    # fi

    return 0
}
switch_kde_color_file(){
  if [ $KDE_COLOR_TAG -eq "1" ]; then
        KDE_COLOR_TAG="2"
         else
          KDE_COLOR_TAG="1"
     fi
}
# 处理壁纸变化
process_wallpaper_change() {
    log_info "检测到壁纸变化"

    # 获取当前壁纸
    local wallpaper_path="$(get_current_wallpaper)"
    if [ -z "$wallpaper_path" ]; then
        log_error "无法获取壁纸路径"
        return 1
    fi

    log_info "当前壁纸: $wallpaper_path"

    # 检查是否与上次相同（避免重复处理）
    local last_wallpaper_file="${SCRIPT_DIR}/.last_wallpaper"
    local last_wallpaper=""
    if [ -f "$last_wallpaper_file" ]; then
        last_wallpaper="$(cat "$last_wallpaper_file")"
    fi

    if [ "$wallpaper_path" = "$last_wallpaper" ]; then
#        log_info "壁纸未变化，跳过处理"
        return 0
    fi

    echo "$wallpaper_path" > "$last_wallpaper_file"

    # 生成配色
    if ! run_pywal "$wallpaper_path"; then
        return 1
    fi

    switch_kde_color_file

    # 更新.colors文件
    if ! update_colors_file; then
        return 1
    fi

    # 应用颜色主题
    if ! apply_color_scheme; then
        return 1
    fi

    log_success "壁纸配色处理完成"
    return 0
}

# 监控壁纸变化
monitor_wallpaper() {
    log_info "开始监控壁纸变化..."

    log_info "轮询间隔: $POLL_INTERVAL_SECONDS 秒"

    # 初始处理
    process_wallpaper_change

    # 尝试使用D-Bus监听（推荐）
    if command -v dbus-monitor &> /dev/null && \
       command -v qdbus6 &> /dev/null; then
        log_info "使用D-Bus监听 + 轮询组合监控..."

        # 启动dbus-monitor进程
        dbus-monitor --session \
            "type='signal',interface='org.kde.PlasmaShell',member='wallpaperChanged'" \
            2>/dev/null | while true; do
            # 使用read -t设置超时，超时后执行轮询检查
            if read -t $POLL_INTERVAL_SECONDS -r line; then
                # 读取到DBus信号
                if [[ "$line" == *"wallpaperChanged"* ]]; then
                    log_info "检测到壁纸变化信号 (DBus)"
                    sleep 0.5  # 等待壁纸完全更新
                    process_wallpaper_change
                fi
            else
                # 超时 - 执行轮询检查
                log_info "轮询检查壁纸变化..."
                process_wallpaper_change
            fi
        done
    else
        # D-Bus工具不可用，使用纯轮询
        log_warn "D-Bus工具不可用，使用纯轮询监控 (间隔: $POLL_INTERVAL_SECONDS 秒)"

        while true; do
            sleep $POLL_INTERVAL_SECONDS
            log_info "轮询检查壁纸变化..."
            process_wallpaper_change
        done
    fi
}

# 显示帮助
show_help() {
    cat << EOF
dycolor-theme-manager - 动态壁纸配色管理器

用法: $0 [选项]

选项:
  --start     启动壁纸监控守护进程
  --stop      停止守护进程
  --once      立即处理当前壁纸
  --status    显示状态
  --help      显示此帮助信息

示例:
  $0 --start      # 启动监控
  $0 --once       # 立即生成配色
  $0 --stop       # 停止监控

配置文件: $CONFIG_FILE
日志文件: $LOG_FILE

配置文件格式:
  第一行: .colors文件绝对路径
  后续行: KDE颜色属性=pywal颜色索引
  可选: [dycolor]poll_interval_seconds=1800 (轮询间隔，单位秒，默认1800，大于0)
  例如:
  /home/user/.local/share/color-themes/DynamicWallpaper.colors
  [Colors:Button]BackgroundNormal=0
  [Colors:Button]ForegroundNormal=7
  [Colors:View]BackgroundNormal=0

EOF
}

# 主函数
main() {
    case "$1" in
        --start)
            check_dependencies || exit 1
            read_config || exit 1

            # 检查是否已在运行
            if [ -f "$PID_FILE" ]; then
                local pid="$(cat "$PID_FILE")"
                if kill -0 "$pid" 2>/dev/null; then
                    log_info "守护进程已在运行 (PID: $pid)"
                    exit 0
                else
                    rm -f "$PID_FILE"
                fi
            fi

            # 启动监控
            log_info "启动动态壁纸配色管理器..."
            monitor_wallpaper &
            local pid=$!
            echo "$pid" > "$PID_FILE"
            log_success "守护进程已启动 (PID: $pid)"
            ;;

        --stop)

            ;;

        --once)
            check_dependencies || exit 1
            read_config || exit 1
            process_wallpaper_change
            ;;

        --status)
            if [ -f "$PID_FILE" ]; then
                local pid="$(cat "$PID_FILE")"
                if kill -0 "$pid" 2>/dev/null; then
                    log_info "守护进程运行中 (PID: $pid)"
                else
                    rm -f "$PID_FILE"
                    log_info "守护进程未运行"
                fi
            else
                log_info "守护进程未运行"
            fi

            # 显示配置信息
            if [ -f "$CONFIG_FILE" ]; then
                log_info "配置文件: $CONFIG_FILE"
                log_info "目标.colors文件: $(head -1 "$CONFIG_FILE")"
            fi

            # 显示上次处理的壁纸
            local last_wallpaper_file="${SCRIPT_DIR}/.last_wallpaper"
            if [ -f "$last_wallpaper_file" ]; then
                log_info "上次处理的壁纸: $(cat "$last_wallpaper_file")"
            fi
            ;;

        --help|-h|"")
            show_help
            ;;

        *)
            log_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

# 运行主函数
main "$@"
