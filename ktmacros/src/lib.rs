use proc_macro::{TokenStream};
use quote::{quote, format_ident};
use syn::{parse_macro_input, ItemFn, LitStr};

#[proc_macro_attribute]
pub fn export_symbol(_attr: TokenStream, item: TokenStream) -> TokenStream {
    let input = parse_macro_input!(item as ItemFn);
    let name = &input.sig.ident;

    let symtab_name = format_ident!("__ktsymbols_{}", name);
    
    let name_str = name.to_string(); 
    let name_literal = format!("{}\0", name_str);

    let expanded = quote! {
        #[unsafe(no_mangle)]
        #input

        #[used]
        #[unsafe(link_section = "___ktsymbols")]
        pub static #symtab_name: KernelSymbol = KernelSymbol {
            value: SyncPtr(#name as *const()),
            name: SyncPtr(#name_literal.as_ptr()),
        };
    };

    TokenStream::from(expanded)
}

#[proc_macro_attribute]
pub fn kmodule_entry(attr: TokenStream, item: TokenStream) -> TokenStream {
    let driver_name = parse_macro_input!(attr as LitStr);
    let input_fn = parse_macro_input!(item as ItemFn);
    let entry_point_ident = &input_fn.sig.ident;

    let expanded = quote! {
        #input_fn

        #[unsafe(no_mangle)]
        #[used]
        #[unsafe(link_section = ".ktdrv")]
        pub static kt_driver_entry: KtModuleEntry = #entry_point_ident;

        #[unsafe(no_mangle)]
        #[used]
        #[unsafe(link_section = ".ktdrv")]
        pub static kt_driver_name: SyncPtr = SyncPtr(concat!(#driver_name, "\0").as_ptr());
    };

    TokenStream::from(expanded)
}