var m_ProfileItem;
var gCustomFilaments = [];
var gCustomTypeFilters = new Set();
var gCustomVendorFilters = new Set();
var gCustomTypeFiltersInitialized = false;
var gCustomVendorFiltersInitialized = false;

var FilamentPriority=new Array( "pla","abs","pet","tpu","pc");
var VendorPriority=new Array("generic");

function OnInit()
{
	TranslatePage();
    OnSelectMenu(1);
	document.addEventListener('click', function(e){
		if(!e.target.closest('.CF_FilterDropdown')){
			CloseCustomFilterMenus();
		}
	});
	
	RequestProfile();
	
	RequestCustomFilaments();
	//TestCustomFilaments();
	//OnSelectMenu(2);
}

function RequestProfile()
{
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="request_userguide_profile";
	
	SendWXMessage( JSON.stringify(tSend) );
}

function HandleStudio(pVal)
{
	let strCmd=pVal['command'];
	//alert(strCmd);
	
	if(strCmd=='response_userguide_profile')
	{
		m_ProfileItem=pVal['response'];
		SortUI();
	}
	else if(strCmd=='update_custom_filaments')
	{
		UpdateCustomFilaments( pVal['data'] );
	}
}

function GetFilamentShortname( sName )
{
	let sShort=sName.split('@')[0].trim();
	
	return sShort;
}


function SortUI()
{
	var ModelList=new Array();
	
	let nMode=m_ProfileItem["model"].length;
	for(let n=0;n<nMode;n++)
	{
		let OneMode=m_ProfileItem["model"][n];
		
		if( OneMode["nozzle_selected"]!="" )
			ModelList.push(OneMode);
	}
	

	//model
	let HtmlMode='';
	nMode=ModelList.length;
	for(let n=0;n<nMode;n++)
	{
		let sModel=ModelList[n];	
		/* ORCA use label tag to allow checkbox to toggle when user ckicked to text */
		HtmlMode+='<label><input type="checkbox" mode="'+sModel['model']+'"  nozzle="'+sModel['nozzle_selected']+'"   onChange="MachineClick()" /><span>'+sModel['model']+'</span></label>';
	}
	
	$('#MachineList .CValues').append(HtmlMode);	
	$('#MachineList .CValues input').prop("checked",true);
	if(nMode<=1)
	{
		$('#MachineList').hide();
	}
	
	//Filament - Create sorted array with generic vendor first
	let FilamentArray=new Array();
	let GenericFilamentArray=new Array();
	for( let key in m_ProfileItem['filament'] )
	{
		let OneFila=m_ProfileItem['filament'][key];
		if(OneFila['vendor'].toLowerCase() === 'generic')
			GenericFilamentArray.push({key: key, data: OneFila});
		else
			FilamentArray.push({key: key, data: OneFila});
	}
	// Combine arrays with generic filaments first
	let SortedFilamentArray = GenericFilamentArray.concat(FilamentArray);
	
	let HtmlFilament='';
	let SelectNumber=0;

	var TypeHtmlArray={};
    var VendorHtmlArray={};
	for( let n=0; n<SortedFilamentArray.length; n++ )
	{
		let filamentItem = SortedFilamentArray[n];
		let key = filamentItem.key;
		let OneFila = filamentItem.data;
		
		//alert(JSON.stringify(OneFila));
		
		let fWholeName=OneFila['name'].trim();
		let fShortName=GetFilamentShortname( OneFila['name'] );
		let fVendor=OneFila['vendor'];
		let fType=OneFila['type'];
		let fSelect=OneFila['selected'];
		let fModel=OneFila['models']
		
        let bFind=false;		
		//let bCheck=$("#MachineList input:first").prop("checked");
		if( fModel=='')
		{
			bFind=true;
		}
		else
		{
			//check in modellist		    
		    let nModelAll=ModelList.length;
		    for(let m=0;m<nModelAll;m++)
		    {
	    		let sOne=ModelList[m];
			
				let OneName=sOne['model'];
				let NozzleArray=sOne["nozzle_selected"].split(';');
				
				let nNozzle=NozzleArray.length;
				
				for( let b=0;b<nNozzle;b++ )
				{
					let nowModel= OneName+"++"+NozzleArray[b];
					if(fModel.indexOf(nowModel)>=0)
					{
						bFind=true;
						break;
					}
				}
			}
		}
		
		if(bFind)
		{
			//Type
			let LowType=fType.toLowerCase();
		    if(!TypeHtmlArray.hasOwnProperty(LowType))
		    {
				/* ORCA use label tag to allow checkbox to toggle when user ckicked to text */
			    let HtmlType='<label><input type="checkbox" filatype="'+fType+'" onChange="FilaClick()"   /><span>'+fType+'</span></label>';
			
				TypeHtmlArray[LowType]=HtmlType;
		    }
			
			//Vendor
			let lowVendor=fVendor.toLowerCase();
			if(!VendorHtmlArray.hasOwnProperty(lowVendor))
		    {
				/* ORCA use label tag to allow checkbox to toggle when user ckicked to text */
			    let HtmlVendor='<label><input type="checkbox" vendor="'+fVendor+'"  onChange="VendorClick()" /><span>'+fVendor+'</span></label>';
				
				VendorHtmlArray[lowVendor]=HtmlVendor;
		    }
			
			//Filament
			let pFila=$("#ItemBlockArea input[vendor='"+fVendor+"'][filatype='"+fType+"'][name='"+fShortName+"']");
	        if(pFila.length==0)
		    {
				/* ORCA use label tag to allow checkbox to toggle when user ckicked to text */
			    let HtmlFila='<label class="MItem"><input type="checkbox" vendor="'+fVendor+'"  filatype="'+fType+'" filalist="'+fWholeName+';'+'"  model="'+fModel+'" name="'+fShortName+'" /><span>'+fShortName+'</span></label>';
			
			    $("#ItemBlockArea").append(HtmlFila);
		    } 
			else
			{
				let strModel=pFila.attr("model");
				let strFilalist=pFila.attr("filalist");
				
				if(strModel == '' || fModel == '')
					pFila.attr("model", '');
				else
					pFila.attr("model", strModel+fModel);
				pFila.attr("filalist", strFilalist+fWholeName+';');
			}
			
		    if(fSelect*1==1)
			{
				//alert( fWholeName+' - '+fShortName+' - '+fVendor+' - '+fType+' - '+fSelect+' - '+fModel );
					
				$("#ItemBlockArea input[vendor='"+fVendor+"'][filatype='"+fType+"'][name='"+fShortName+"']").prop("checked",true);
				SelectNumber++;
			}
//			else
//				$("#ItemBlockArea input[vendor='"+fVendor+"'][model='"+fModel+"'][filatype='"+fType+"'][name='"+key+"']").prop("checked",false);			
		}
	} 

	//Sort TypeArray
	let TypeAdvNum=FilamentPriority.length;
	for( let n=0;n<TypeAdvNum;n++ )
	{
		let strType=FilamentPriority[n];
		
		if( TypeHtmlArray.hasOwnProperty( strType ) )
		{
			$("#FilatypeList .CValues").append( TypeHtmlArray[strType] );
			delete( TypeHtmlArray[strType] );
		}
	}
    for(let key in TypeHtmlArray )
	{
		$("#FilatypeList .CValues").append( TypeHtmlArray[key] );
	}
	$("#FilatypeList .CValues input").prop("checked",true);
	
	//Sort VendorArray
	let VendorAdvNum=VendorPriority.length;
	for( let n=0;n<VendorAdvNum;n++ )
	{
		let strVendor=VendorPriority[n];
		
		if( VendorHtmlArray.hasOwnProperty( strVendor ) )
		{
			$("#VendorList .CValues").append( VendorHtmlArray[strVendor] );
			delete( VendorHtmlArray[strVendor] );
		}
	}
    for(let key in VendorHtmlArray )
	{
		$("#VendorList .CValues").append( VendorHtmlArray[key] );
	}	
	$("#VendorList .CValues input").prop("checked",true);
	
	//------
	if(SelectNumber==0)
		ChooseDefaultFilament();
}


function ChooseAllMachine()
{
	let bCheck=$("#MachineList input:first").prop("checked");
	
	$("#MachineList input").prop("checked",bCheck);
	
	SortFilament();
}

function MachineClick()
{
	let nChecked=$("#MachineList input:gt(0):checked").length
	let nAll    =$("#MachineList input:gt(0)").length
	
	if(nAll==nChecked)
	{
		$("#MachineList input:first").prop("checked",true);
	}
	else
	{
		$("#MachineList input:first").prop("checked",false);
	}
	
	SortFilament();
}

function ChooseAllFilament()
{
    let bCheck=$("#FilatypeList input:first").prop("checked");	
	$("#FilatypeList input").prop("checked",bCheck);	
    
    SortFilament();
}

function FilaClick()
{
	let nChecked=$("#FilatypeList input:gt(0):checked").length
	let nAll    =$("#FilatypeList input:gt(0)").length
	
	if(nAll==nChecked)
	{
		$("#FilatypeList input:first").prop("checked",true);
	}
	else
	{
		$("#FilatypeList input:first").prop("checked",false);
	}
	
	SortFilament();	
}

function ChooseAllVendor()
{
	let bCheck=$("#VendorList input:first").prop("checked");	
	$("#VendorList input").prop("checked",bCheck);	
	
	SortFilament();
}

function VendorClick()
{
	let nChecked=$("#VendorList input:gt(0):checked").length
	let nAll    =$("#VendorList input:gt(0)").length
	
	if(nAll==nChecked)
	{
		$("#VendorList input:first").prop("checked",true);
	}
	else
	{
		$("#VendorList input:first").prop("checked",false);
	}
	
	SortFilament();
}



function SortFilament()
{
	let FilaNodes=$("#ItemBlockArea .MItem");
	let nFilament=FilaNodes.length;
	//$("#ItemBlockArea .MItem").hide();
	
	//ModelList
	let pModel=$("#MachineList input:checked");
	let nModel=pModel.length;
	let ModelList=new Array();
	for(let n=0;n<nModel;n++)
	{
		let OneModel=pModel[n];
		
		let mName=OneModel.getAttribute("mode");
		if( mName=='all' )
		{
			continue;
		}
		else
		{
			let mNozzle=OneModel.getAttribute("nozzle");
			let NozzleArray=mNozzle.split(';');
			
			for( let bb=0;bb<NozzleArray.length;bb++ )
			{
				let NewModel='['+mName+'++'+NozzleArray[bb]+']';
			
				ModelList.push( NewModel );
			}
		}
	}
	
	//TypeList
	let pType=$("#FilatypeList input:gt(0):checked");
	let nType=pType.length;
	let TypeList=new Array();
	for(let n=0;n<nType;n++)
	{
		let OneType=pType[n];
		TypeList.push(  OneType.getAttribute("filatype") );
	}	
	
	//VendorList
	let pVendor=$("#VendorList input:gt(0):checked");
	let nVendor=pVendor.length;
	let VendorList=new Array();
	for(let n=0;n<nVendor;n++)
	{
		let OneVendor=pVendor[n];
		VendorList.push(  OneVendor.getAttribute("vendor") );
	}		
	
	
	//Update Filament UI
	for(let m=0;m<nFilament;m++)
	{
		let OneNode=FilaNodes[m];
		let OneFF=OneNode.getElementsByTagName("input")[0];
		
	    let fModel=OneFF.getAttribute("model");
		let fVendor=OneFF.getAttribute("vendor");
		let fType=OneFF.getAttribute("filatype");
		let fName=OneFF.getAttribute("name");
		
		if(TypeList.in_array(fType) && VendorList.in_array(fVendor))
		{
			let HasModel=false;
			for(let m=0;m<ModelList.length;m++)
			{
				let ModelSrc=ModelList[m];
				
				if( fModel.indexOf(ModelSrc)>=0)
				{
					HasModel=true;
					break;
				}
			}
			
			if(HasModel || fModel=='')
			    $(OneNode).show();
			else
				$(OneNode).hide();
		}
		else
			$(OneNode).hide();
	}
}

function ChooseDefaultFilament()
{
	//ModelList
	let pModel=$("#MachineList input:gt(0):checked");
	let nModel=pModel.length;
	let ModelList=new Array();
	for(let n=0;n<nModel;n++)
	{
		let OneModel=pModel[n];
		ModelList.push(  OneModel.getAttribute("mode") );
	}	
	
	//Filament
	let FilaNodes=$("#ItemBlockArea .MItem");
    let nFilament=FilaNodes.length;
    for(let m=0;m<nFilament;m++)
	{
		let OneNode=FilaNodes[m];
		let OneFF=OneNode.getElementsByTagName("input")[0];
		$(OneFF).prop("checked",false);
		
	    let fModel=OneFF.getAttribute("model");
		
		let HasModel=false;
		for(let m=0;m<nModel;m++)
		{
			let ModelSrc=ModelList[m];
		
			if( fModel.indexOf(ModelSrc)>=0)
			{
				HasModel=true;
				break;
			}
		}
			
		if(HasModel)
		    $(OneFF).prop("checked",true);
	}
	
	ShowNotice(0);
}

function SelectAllFilament( nShow )
{
	// ORCA add ability to only select / unselect filted items
	if (document.querySelector('.cbr-filter-bar').value) {
		$('#ItemBlockArea .MItem:visible input')
		.filter(function() {return $(this).closest('.MItem').css('position') !== 'absolute'})
		.prop("checked", nShow != 0);
	}
	else {
		$('#ItemBlockArea .MItem:visible input').prop("checked",nShow!=0);
	}
}

function ShowNotice( nShow )
{
	if(nShow==0)
	{
		$("#NoticeMask").hide();
		$("#NoticeBody").hide();
	}
	else
	{
		$("#NoticeMask").show();
		$("#NoticeBody").show();
	}
}


function ResponseFilamentResult()
{
	let FilaSelectedList= $("#ItemBlockArea input:checked");
	let nAll=FilaSelectedList.length;

	if( nAll==0 )
	{
		ShowNotice(1);
		return false;
	}
	
	let FilaArray=new Array();
	for(let n=0;n<nAll;n++)
	{
		let strFilalist=FilaSelectedList[n].getAttribute("filalist");
		if(strFilalist) {
			let filaNames = strFilalist.split(';');
			for(let i=0; i<filaNames.length; i++) {
				let fname = filaNames[i].trim();
				if(fname !== '')
					FilaArray.push(fname);
			}
		}
	}
	
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="save_userguide_filaments";
	tSend['data']={};
	tSend['data']['filament']=FilaArray;
	
	SendWXMessage( JSON.stringify(tSend) );
	
	return true;
}


function CancelSelect()
{
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="user_guide_cancel";
	tSend['data']={};
		
	SendWXMessage( JSON.stringify(tSend) );			
}


function ConfirmSelect()
{
	let bRet=ResponseFilamentResult();
	
	if(bRet)
    {
		var tSend={};
		tSend['sequence_id']=Math.round(new Date() / 1000);
		tSend['command']="user_guide_finish";
		tSend['data']={};
		tSend['data']['action']="finish";
		
		SendWXMessage( JSON.stringify(tSend) );			
	}
}


function OnSelectMenu( nIndex )
{
	switch(nIndex)
	{
		case 1:
			$('#SystemFilamentBtn').addClass('TitleSelected');
			$('#SystemFilamentBtn').removeClass('TitleUnselected');		
			
			$('#CustomFilamentBtn').addClass('TitleUnselected');
			$('#CustomFilamentBtn').removeClass('TitleSelected');	
			
			$('#SystemFilamentsArea').css('display','flex');
			$('#CustomFilamentsArea').css('display','none');
			break;
		case 2:
			$('#CustomFilamentBtn').addClass('TitleSelected');
			$('#CustomFilamentBtn').removeClass('TitleUnselected');
			
			$('#SystemFilamentBtn').addClass('TitleUnselected');
			$('#SystemFilamentBtn').removeClass('TitleSelected');	
			
			$('#CustomFilamentsArea').css('display','flex');
			$('#SystemFilamentsArea').css('display','none');			
			break;
	}
}

function RequestCustomFilaments()
{
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="request_custom_filaments";
		
	SendWXMessage( JSON.stringify(tSend) );		
}

function TestCustomFilaments()
{
	let strTest='{"command":"update_custom_filaments","data":[{"id":"P0c71f94","name":"AMOLEN ABS 222"},{"id":"P19cc6c5","name":"PrimaSelect PLA 231654"},{"id":"P93a5c3b","name":"3DJAKE PLA 111"}],"sequence_id":"2000"}';
	let tItem=JSON.parse(strTest);
	
	HandleStudio(tItem);
}

function UpdateCustomFilaments( CFList )
{
	gCustomFilaments = (CFList || []).map(function(pItem){
		return {
			id: pItem['id'] || '',
			name: pItem['name'] || '',
			type: pItem['type'] || 'Unknown',
			vendor: pItem['vendor'] || 'Unknown',
			origin_status: pItem['origin_status'] || '',
			parent_status: pItem['parent_status'] || '',
			cloud_sync_status: pItem['cloud_sync_status'] || ''
		};
	});
	gCustomFilaments.sort(function(a,b){
		let at=(a.type||'').toLowerCase(), bt=(b.type||'').toLowerCase();
		if(at!==bt) return at<bt ? -1 : 1;
		let av=(a.vendor||'').toLowerCase(), bv=(b.vendor||'').toLowerCase();
		if(av!==bv) return av<bv ? -1 : 1;
		let an=(a.name||'').toLowerCase(), bn=(b.name||'').toLowerCase();
		if(an===bn) return 0;
		return an<bn ? -1 : 1;
	});
	RefreshCustomFilterMenus();
	RenderCustomFilamentRows();
	FilterCustomFilaments();
}

function RenderCustomFilamentRows()
{
	let strHtml='';
	let nTotal=gCustomFilaments.length;
	for(let n=0;n<nTotal;n++)
	{
		let pItem=gCustomFilaments[n];
		let F_id=pItem.id;
		let F_name=pItem.name;
		let F_type=pItem.type || 'Unknown';
		let F_vendor=pItem.vendor || 'Unknown';
		let F_name_attr=EncodeAttr(F_name);
		let F_type_attr=EncodeAttr(F_type);
		let F_vendor_attr=EncodeAttr(F_vendor);
		let originStatus=pItem.origin_status;
		let parentStatus=pItem.parent_status;
		let cloudStatus=pItem.cloud_sync_status;
		let statusText=[originStatus,parentStatus,cloudStatus,F_type,F_vendor].filter(Boolean).join(' ');
		let statusHtml='';
		if(originStatus) statusHtml += '<span class="CF_Badge">'+FormatOriginStatus(originStatus)+'</span>';
		if(parentStatus) statusHtml += '<span class="CF_Badge">'+FormatParentStatus(parentStatus)+'</span>';
		if(cloudStatus)  statusHtml += '<span class="CF_Badge">'+FormatCloudStatus(cloudStatus)+'</span>';
		let strAdd='<div class="CFilament_Item" data-type="'+F_type_attr+'" data-vendor="'+F_vendor_attr+'" data-filter="'+EncodeAttr((F_name + " " + statusText).toLowerCase())+'">'+
			       '<div class="CFilament_Row"><span class="CFilament_Type" title="'+F_type_attr+'">'+F_type_attr+'</span><a class="CFilament_Name" title="'+F_name_attr+'">'+F_name_attr+'</a><img data-id="'+EncodeAttr(F_id)+'" data-name="'+F_name_attr+'" onClick="CFEdit(this)" class="CFilament_EditBtn" src="../../image/edit.svg" /><button class="CFilament_DeleteBtn" data-id="'+EncodeAttr(F_id)+'" data-name="'+F_name_attr+'" onClick="CFDelete(this)">Delete</button></div>'+
				   '<div class="CFilament_Status">'+statusHtml+'</div>'+
				   '</div>';
		strHtml+=strAdd;
	}
	$('#CFilament_List').html(strHtml);
}

function RefreshCustomFilterMenus()
{
	let typeValues=[];
	let vendorValues=[];
	let typeMap={};
	let vendorMap={};
	for(let i=0;i<gCustomFilaments.length;i++){
		let t=(gCustomFilaments[i].type||'').trim();
		let v=(gCustomFilaments[i].vendor||'').trim();
		if(t && !typeMap[t]){ typeMap[t]=1; typeValues.push(t); }
		if(v && !vendorMap[v]){ vendorMap[v]=1; vendorValues.push(v); }
	}
	typeValues.sort();
	vendorValues.sort();
	if(!gCustomTypeFiltersInitialized){
		typeValues.forEach(v=>gCustomTypeFilters.add(v));
		gCustomTypeFiltersInitialized = true;
	}
	if(!gCustomVendorFiltersInitialized){
		vendorValues.forEach(v=>gCustomVendorFilters.add(v));
		gCustomVendorFiltersInitialized = true;
	}
	gCustomTypeFilters = new Set([...gCustomTypeFilters].filter(v=>typeMap[v]));
	gCustomVendorFilters = new Set([...gCustomVendorFilters].filter(v=>vendorMap[v]));
	RenderCustomFilterMenu('type', typeValues, gCustomTypeFilters);
	RenderCustomFilterMenu('vendor', vendorValues, gCustomVendorFilters);
	UpdateCustomFilterButtonTitles(typeValues, vendorValues);
}

function RenderCustomFilterMenu(kind, values, selectedSet)
{
	let menu = kind==='type' ? $('#CF_Type_Filter_Menu') : $('#CF_Vendor_Filter_Menu');
	let html='<label><input type="checkbox" data-kind="'+kind+'" data-value="__all__" '+(selectedSet.size===values.length?'checked':'')+' onchange="OnCustomFilterAllToggle(this)" />All</label>';
	for(let i=0;i<values.length;i++){
		let val=values[i];
		html+='<label><input type="checkbox" data-kind="'+kind+'" data-value="'+EncodeAttr(val)+'" '+(selectedSet.has(val)?'checked':'')+' onchange="OnCustomFilterToggle(this)" />'+EncodeAttr(val)+'</label>';
	}
	menu.html(html);
}

function UpdateCustomFilterButtonTitles(typeValues, vendorValues)
{
	let tTitle=(gCustomTypeFilters.size===typeValues.length)?'Type':'Type ('+gCustomTypeFilters.size+')';
	let vTitle=(gCustomVendorFilters.size===vendorValues.length)?'Vendor':'Vendor ('+gCustomVendorFilters.size+')';
	$('#CF_Type_Filter_Btn').text(tTitle);
	$('#CF_Vendor_Filter_Btn').text(vTitle);
}

function ToggleCustomFilterMenu(kind)
{
	let menu = kind==='type' ? $('#CF_Type_Filter_Menu') : $('#CF_Vendor_Filter_Menu');
	let btn  = kind==='type' ? $('#CF_Type_Filter_Btn') : $('#CF_Vendor_Filter_Btn');
	let isOpen = menu.hasClass('open');
	CloseCustomFilterMenus();
	if(!isOpen){
		menu.addClass('open');
		btn.addClass('open');
	}
}

function CloseCustomFilterMenus()
{
	$('#CF_Type_Filter_Menu,#CF_Vendor_Filter_Menu').removeClass('open');
	$('#CF_Type_Filter_Btn,#CF_Vendor_Filter_Btn').removeClass('open');
}

function OnCustomFilterAllToggle(cb)
{
	let kind = cb.getAttribute('data-kind');
	let checked = cb.checked;
	let values = [];
	if(kind==='type'){
		$('#CF_Type_Filter_Menu input[data-value!="__all__"]').each(function(){ values.push(this.getAttribute('data-value')); this.checked=checked; });
		gCustomTypeFilters = new Set(checked ? values : []);
	}else{
		$('#CF_Vendor_Filter_Menu input[data-value!="__all__"]').each(function(){ values.push(this.getAttribute('data-value')); this.checked=checked; });
		gCustomVendorFilters = new Set(checked ? values : []);
	}
	RefreshCustomFilterMenus();
	FilterCustomFilaments();
}

function OnCustomFilterToggle(cb)
{
	let kind = cb.getAttribute('data-kind');
	let value = cb.getAttribute('data-value');
	if(kind==='type'){
		if(cb.checked) gCustomTypeFilters.add(value); else gCustomTypeFilters.delete(value);
	}else{
		if(cb.checked) gCustomVendorFilters.add(value); else gCustomVendorFilters.delete(value);
	}
	RefreshCustomFilterMenus();
	FilterCustomFilaments();
}

function FilterCustomFilaments()
{
	let search=$('#CFilament_Filter').val();
	if(!search) search='';
	search=search.trim().toLowerCase();
	$('#CFilament_List .CFilament_Item').each(function(){
		let hay=$(this).attr('data-filter') || '';
		let type=$(this).attr('data-type') || '';
		let vendor=$(this).attr('data-vendor') || '';
		let typeMatch=(gCustomTypeFilters.size===0) ? false : gCustomTypeFilters.has(type);
		let vendorMatch=(gCustomVendorFilters.size===0) ? false : gCustomVendorFilters.has(vendor);
		if((!search || hay.indexOf(search)>=0) && typeMatch && vendorMatch)
			$(this).show();
		else
			$(this).hide();
	});
}

function EncodeAttr(str)
{
	return String(str || '')
		.replace(/&/g, '&amp;')
		.replace(/"/g, '&quot;')
		.replace(/'/g, '&#39;')
		.replace(/</g, '&lt;')
		.replace(/>/g, '&gt;');
}

function FormatOriginStatus(originStatus)
{
	switch(originStatus){
		case 'detached': return 'Detached';
		case 'inherited': return 'Inherited';
		case 'new': return 'New';
		default: return originStatus;
	}
}

function FormatParentStatus(parentStatus)
{
	switch(parentStatus){
		case 'from_generic': return 'From Generic';
		case 'from_system': return 'From System';
		case 'from_user': return 'From User';
		default: return parentStatus;
	}
}

function FormatCloudStatus(cloudStatus)
{
	switch(cloudStatus){
		case 'synced': return 'Cloud Synced';
		case 'pending': return 'Cloud Pending';
		case 'hold': return 'Cloud Hold';
		case 'local_only': return 'Cloud Local';
		default: return cloudStatus;
	}
}


function OnClickCustomFilamentAdd()
{
	//alert('Create New Custom Filament');
	
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="create_custom_filament";
		
	SendWXMessage( JSON.stringify(tSend) );		
}

//编辑某一个自定义材料
function CFEdit( btn )
{
	let fid=$(btn).attr('data-id');
	let fname=$(btn).attr('data-name');
	
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="modify_custom_filament";
	tSend['id']=fid;
	tSend['name']=fname;
		
	SendWXMessage( JSON.stringify(tSend) );	
}

function CFDelete( btn )
{
	let fid=$(btn).attr('data-id');
	let fname=$(btn).attr('data-name');
	var tSend={};
	tSend['sequence_id']=Math.round(new Date() / 1000);
	tSend['command']="delete_custom_filament";
	tSend['id']=fid;
	tSend['name']=fname;
		
	SendWXMessage( JSON.stringify(tSend) );
}
