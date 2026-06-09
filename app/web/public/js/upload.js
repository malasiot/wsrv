$(document).ready(function() {
    // Show form, hide dashboard layout
    $('#btnNewRoute').on('click', function() {
        $('#dashboardListView').fadeOut(200, function() {
            $('#formContainer').fadeIn(200);
        });
    });

    // Hide form, return to dashboard layout
    $('#btnCancelRoute, #btnCancelBottom').on('click', function() {
        $('#formContainer').fadeOut(200, function() {
            $('#dashboardListView').fadeIn(200);
            $('#newRouteForm')[0].reset(); 
        });
    });

    // AJAX Handler
    $('#newRouteForm').on('submit', function(e) {
        e.preventDefault();
        var formData = new FormData(this);
        var $submitBtn = $('#btnSubmitRoute');
        
        $submitBtn.prop('disabled', true).html('<span class="spinner-border spinner-border-sm" role="status" aria-hidden="true"></span> {{ "upload.saving"|trans }}...');

        $.ajax({
            url: '/api/routes/create',
            type: 'POST',
            data: formData,
            processData: false, 
            contentType: false, 
            success: function(response) {
                $('#newRouteForm')[0].reset();
                $('#formContainer').hide();
                $('#dashboardListView').show();
                // Optional: trigger dynamic list reload here
            },
            error: function(xhr) {
                alert('Error: ' + xhr.responseText);
            },
            complete: function() {
                $submitBtn.prop('disabled', false).text("{{ 'route.form.save'|trans }}");
            }
        });
    });
});