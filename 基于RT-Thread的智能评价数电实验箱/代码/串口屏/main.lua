------------------------------------
-- ��ʾ
-- ���ʹ������Lua�༭���߱༭���ĵ����뽫VisualTFT����д򿪵Ĵ��ļ��༭����ͼ�رգ�
-- ��ΪVisualTFT�����Զ����湦�ܣ���������޸�ʱ����ͬ����VisualTFT�༭��ͼ��
-- VisualTFT��ʱ����ʱ����������޸ĵ����ݽ����ָ���
--
-- Attention
-- If you use other Lua Editor to edit this file, please close the file editor view 
-- opened in the VisualTFT, Because VisualTFT has an automatic save function, 
-- other Lua Editor cannot be synchronized to the VisualTFT edit view when it is modified.
-- When VisualTFT saves regularly, the content modified by other Lua Editor will be restored.
------------------------------------

--�����г��˳��õĻص�����
--���๦�����Ķ�<<LUA�ű�API.pdf>>


--��ʼ������
--function on_init()
local number = 0
local count=0
local light_value=0
local sound_value=0
local time=0
local flag=0
local hour=0
local minute=0
local second=0
local cmd_head    = 0xB5
local cmd_end     = 0xFFFCFFFF
local ht_cmd_end     = 0x30313233
local buff = {}
local cmd_length = 0
local cmd_end_tag      = 0
local sy_flag=0


--end

--��ʱ�ص�������ϵͳÿ��1�����Զ����á�
function on_systick()
--ˢ�������ʼ��ʱ
	if flag==1 then
	time=time+1
	hour=math.floor(time/3600)
	minute=math.floor((time-(hour*3600))/60)
	second=math.floor(time-(hour*3600)-(minute*60))
	local formatted_string = string.format("%02d:%02d:%02d", hour, minute, second)
		--��ʾʱ����ɰ�
set_text(1,15,formatted_string )	
 set_text(2,5,formatted_string )	
 set_text(3,10,formatted_string )	
 set_text(4,10,formatted_string )	
set_text(6,10,formatted_string )
 set_text(7,10,formatted_string )	 
	
 set_text(9,15,formatted_string )		
  set_text(10,10,formatted_string )	 
	end
end
--��ʱ����ʱ�ص������������õĶ�ʱ����ʱʱ��ִ�д˻ص�������timer_idΪ��Ӧ�Ķ�ʱ��ID
--function on_timer(timer_id)
--end

--�û�ͨ�������޸Ŀؼ���ִ�д˻ص�������
--�����ť�ؼ����޸��ı��ؼ����޸Ļ��������ᴥ�����¼���
function on_control_notify(screen,control,value)
	
	--if screen==1 and control==2 and value==1 then
	--count=count+1
	--set_text(1,3,count)
	--end
	--   if screen==2 and control == 1 then
if screen==1 and control ==7 and value==1 then
		if(sy_flag==2) then
		change_screen(4)
		end
		if(sy_flag==3) then
		change_screen(9)
		end

	
		
	end

	-----------���ڷ���ѡ��ʵ����ͷ 0xEE 0XF5
	
 if screen==6 and control==41 and value==1 then
 set_text(6,1,"  ")	
 set_text(6,2,"  ")
 set_text(6,3,"  ")
 set_text(6,4,"  ")
 set_text(6,5,"  ")
 set_text(6,6,"  ")
 set_text(6,7,"  ") 
  set_text(6,8,"  ") 
 
 end	
	if screen==0 and control==1 and value==1 then--ѡ��ʵ��2
	local uart_data={}
	uart_data[0]=0xD3
	uart_data[1]=0xF5
	uart_data[2]=0xE2
	uart_data[3]=0x33
	uart_data[4]=0x02
 uart_data[5]=0x12
	uart_data[6]=0x34
	sy_flag=2	
	
 uart_send_data(uart_data)	
end

	if screen==0 and control==2 and value==1 then--ѡ��ʵ��1
	local uart_data={}
	uart_data[0]=0xD3
	uart_data[1]=0xF5
	uart_data[2]=0xE2
	uart_data[3]=0x33--ѡ��ʵ��
	uart_data[4]=0x01
    uart_data[5]=0x12
	uart_data[6]=0x34	
 uart_send_data(uart_data)	
end
if screen==0 and control==4 and value==1 then--ѡ��ʵ��3
	local uart_data={}
	uart_data[0]=0xD3
	uart_data[1]=0xF5
	uart_data[2]=0xE2
	uart_data[3]=0x33
	uart_data[4]=0x03
	uart_data[5]=0x12
	uart_data[6]=0x34
	sy_flag=3
	
 uart_send_data(uart_data)	
end


	  if screen==2 and control == 2 then  --������6ҳ�����2������  
        set_backlight(value)             --���ñ���Ϊvalue
 		set_text(2,9,value)
	end
	if screen==2 and control==1 then
		if value==0 then 
			set_volume(0) 		
		end
 		if value==1 then 
			set_volume(100) 		
		end	
	end
	if  screen==0 and control==1 and value==1 then
	flag=1;		
	end

 if  screen==0 and control==4 and value==1 then
	flag=1;		
	end	
end

--�������л�ʱ��ִ�д˻ص�������screenΪĿ�껭�档
--function on_screen_change(screen)
--end

function my_processmessage(msg) 
     
    local funccode = msg[1] 
    if funccode == Func_lampState 
    then 
         
        local xth_lamp = msg[2] 
        local xth_lamp_state = msg[3] 
        my_set_lamp_state(xth_lamp, xth_lamp_state)
    elseif funccode == Func_lampLight 
    then 
        local xth_lamp = msg[2] 
        local xth_lamp_light = msg[3] 
        my_set_lamp_light(xth_lamp, xth_lamp_light) 
    end 
end




function on_uart_recv_data(packet)
	local i
	local recv_packet_size = (#(packet))
	local check16=0
	local cmd_head_tag = 0	

	for i = 0, recv_packet_size 
	do
		if packet[i] == 0xEE and packet[i+1] == 0xB5 and cmd_head_tag == 0
		then
			cmd_head_tag = 1
		end
		
		if cmd_head_tag == 1
		then
			buff[cmd_length] = packet[i]
			cmd_length = cmd_length + 1
			cmd_end_tag = (cmd_end_tag << 8) | (packet[i])
			
			if (cmd_end_tag & cmd_end)== cmd_end
			then
				--check16 = ((buff[cmd_length - 6] << 8) | buff[cmd_length - 5]) & 0xFFFF
				--print('check16 = '..string.format('%04X', check16))
				--print('result = '..string.format('%04X', add_crc16(1, cmd_length - 7, buff)))
				my_processmessage(buff)
				buff  = {}
				cmd_length   = 0
				cmd_end_tag  = 0
				cmd_head_tag = 0
--[[
				if check16 == add_crc16(1, cmd_length - 7, buff)
				then
					
					my_processmessage(buff)
					buff         = {}
					cmd_length   = 0
					cmd_end_tag  = 0
					cmd_head_tag = 0
				else
					buff         = {}
					cmd_length   = 0
					cmd_end_tag  = 0
					cmd_head_tag = 0
				end
--]]
			end
		end
	end
end


function my_processmessage(msg) 
     local p=1
    local funccode = msg[2] 
   -- print('my_processmessage') 
	-- print('msg len = '..(#(msg))) 
if msg[2]==0xa0      --����ID
	 then
 	
    if msg[3]==0x01   
		then
		set_text(5,2,"��*")
		set_text(5,3,"32******37")
 	set_text(5,7,"**רҵ2203")	
		 end 
		 

		if msg[3]==0x02   
		then
		set_text(5,2,"��*")
		set_text(5,3,"32******23")
 	set_text(5,7,"**רҵ2302")		
		 end
		
 	if msg[3]==0x02   
		then
		set_text(5,2,"��**")
		set_text(5,3,"32******16")
 	set_text(5,7,"**רҵ2102")		
		 end	
	end


	if msg[2]==0xa1  --�������
	  then
 	  set_text(6,1,"  ")	
 set_text(6,2,"  ")
 set_text(6,3,"  ")
 set_text(6,4,"  ")
 set_text(6,5,"  ")
 set_text(6,6,"  ")
 set_text(6,7,"  ") 
  set_text(6,8,"  ")  
 	 change_screen(6)  
	    if msg[3]&0x01==0x01--ʵ���1��������
	    then
		if(sy_flag==3) then
		change_screen(10)
		end
		set_text(6,p,"A2 A1 A0Ϊ0 0 0 ʱ״̬����")	
		p=p+1
		end
		
 	  if msg[3]&0x02==0x02--ʵ���2��������
	    then
 		set_text(6,p,"A2 A1 A0Ϊ0 0 1 ʱ״̬����")	
		p=p+1
		end
		
		  if msg[3]&0x04==0x04--ʵ���3��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ0 1 0 ʱ״̬����")	
		p=p+1
	
		end
		
		  if msg[3]&0x08==0x08--ʵ���4��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ0 1 1 ʱ״̬����")	
		p=p+1
		end
		
		  if msg[3]&0x10==0x10--ʵ���5��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ1 0 0 ʱ״̬����")	
		p=p+1
		end
		
		  if msg[3]&0x20==0x20--ʵ���6��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ1 0 1 ʱ״̬����")	
		p=p+1
		end
		
		  if msg[3]&0x40==0x40--ʵ���7��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ1 1 0 ʱ״̬����")	
		p=p+1
		end	

 		  if msg[3]&0x80==0x80--ʵ���8��������
	    then
 	 	set_text(6,p,"A2 A1 A0Ϊ1 1 1 ʱ״̬����")	
		p=p+1
		end		
		p=0
end

if msg[2]==0xa2  --������ȷ
	  then
 	 change_screen(8)
	 local time_now=string.format("%02d:%02d:%02d", hour, minute, second)
	 set_text(8,3,time_now )	
	local uart_data={}
	 uart_data[0]=0xF2
	uart_data[1]=0x3d
	uart_data[2]=hour
	uart_data[3]=minute
	uart_data[4]=second
 uart_data[5]=0x63
	uart_data[6]=0x14	
  
	uart_send_data(uart_data)
	  end

end
